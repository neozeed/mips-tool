#include "r3000.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define getcwd _getcwd
#define chdir _chdir
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#define CFE_MACHINE_R3000 0x0162u
#define GUEST_MEM_SIZE    (64u * 1024u * 1024u)
#define GUEST_STACK_TOP   0x03effff0u
#define TRAP_BASE         0x03f00000u
#define TRAP_STRIDE       0x10u
#define TRAP_LIMIT        0x03f10000u
#define MAX_IMPORTS       256u
#define MAX_SECTIONS      32u
#define VM_BASE           0x00200000u
#define VM_LIMIT          0x03800000u
#define CMDLINE_ADDR      0x03e00000u
#define ENV_ADDR          0x03e10000u
#define MODULE_ADDR       0x03e20000u
#define SI_ADDR           0x03e30000u
#define FILE_HANDLE_BASE  0x100u
#define MAX_FILES         32u
#define MAX_JMP_SLOTS     8u
#define MIPS_TOOL_VERSION  "R5C8"

/* Transitional 1991 NT MIPS COFF: PE-like optional/section headers but no MZ/PE wrapper. */
typedef struct import_rec {
    uint32_t trap;
    uint32_t iat;
    char dll[32];
    char name[96];
    uint16_t hint;
} import_rec_t;

typedef struct jmp_slot {
    int valid;
    uint32_t guest_buf;
    uint32_t gpr[32];
    uint32_t hi;
    uint32_t lo;
    uint32_t return_pc;
} jmp_slot_t;

typedef struct cfe_host {
    uint8_t *mem;
    size_t mem_size;
    import_rec_t imports[MAX_IMPORTS];
    unsigned import_count;
    uint32_t vm_next;
    uint32_t last_error;
    uint32_t command_line;
    uint32_t environment;
    uint32_t module_name;
    FILE *files[MAX_FILES];
    jmp_slot_t jmps[MAX_JMP_SLOTS];
    uint64_t hostcalls;
    uint32_t exit_code;
    char image_path[512];
    char tool_name[64];
    char launcher_path[512];
    uint32_t child_process_handle;
    uint32_t child_thread_handle;
    uint32_t child_exit_code;
    uint32_t next_child_id;
    int verbosity;
} cfe_host_t;

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void wr32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static int range_ok(size_t total, uint32_t off, uint32_t len)
{
    return off <= total && len <= total - off;
}

static int guest_range(const cfe_host_t *h, uint32_t addr, unsigned size)
{
    return addr <= h->mem_size && size <= h->mem_size - addr;
}

static int bus_read(void *opaque, uint32_t paddr, unsigned size, uint32_t *value)
{
    cfe_host_t *h = (cfe_host_t *)opaque;
    const uint8_t *p;
    if (!guest_range(h, paddr, size)) return -1;
    p = h->mem + paddr;
    if (size == 1u) *value = p[0];
    else if (size == 2u) *value = rd16(p);
    else if (size == 4u) *value = rd32(p);
    else return -1;
    return 0;
}

static int bus_write(void *opaque, uint32_t paddr, unsigned size, uint32_t value)
{
    cfe_host_t *h = (cfe_host_t *)opaque;
    uint8_t *p;
    if (!guest_range(h, paddr, size)) return -1;
    p = h->mem + paddr;
    if (size == 1u) p[0] = (uint8_t)value;
    else if (size == 2u) { p[0]=(uint8_t)value; p[1]=(uint8_t)(value>>8); }
    else if (size == 4u) wr32(p, value);
    else return -1;
    return 0;
}

static int read_cstr(const cfe_host_t *h, uint32_t addr, char *dst, size_t cap)
{
    size_t i;
    if (!cap || addr >= h->mem_size) return -1;
    for (i = 0; i + 1 < cap && addr + i < h->mem_size; i++) {
        dst[i] = (char)h->mem[addr + i];
        if (!dst[i]) return 0;
    }
    dst[i < cap ? i : cap - 1] = 0;
    return -1;
}

static int patch_imports(cfe_host_t *h, uint32_t idata_va, uint32_t idata_size)
{
    uint32_t desc;
    unsigned di;
    (void)idata_size;
    for (di = 0; di < 32u; di++) {
        uint32_t lookup, namep, stamp, iat;
        char dll[32];
        unsigned ti;
        desc = idata_va + di * 16u;
        if (!guest_range(h, desc, 16u)) return -1;
        lookup = rd32(h->mem + desc + 0);
        namep  = rd32(h->mem + desc + 4);
        stamp  = rd32(h->mem + desc + 8);
        iat    = rd32(h->mem + desc + 12);
        if (!(lookup | namep | stamp | iat)) break;
        if (!namep || !iat || read_cstr(h, namep, dll, sizeof(dll)) != 0) {
            fprintf(stderr, "MIPS-TOOL: malformed import descriptor %u\n", di);
            return -1;
        }
        if (h->verbosity >= 2)
            printf("MIPS-TOOL: import DLL %s timestamp=%08x iat=%08x\n", dll, stamp, iat);
        for (ti = 0; ti < MAX_IMPORTS; ti++) {
            uint32_t nameent, trap;
            import_rec_t *r;
            if (!guest_range(h, iat + ti * 4u, 4u)) return -1;
            nameent = rd32(h->mem + iat + ti * 4u);
            if (!nameent) break;
            if (h->import_count >= MAX_IMPORTS || !guest_range(h, nameent, 3u)) return -1;
            r = &h->imports[h->import_count];
            memset(r, 0, sizeof(*r));
            memcpy(r->dll, dll, strlen(dll) + 1u);
            r->hint = rd16(h->mem + nameent);
            if (read_cstr(h, nameent + 2u, r->name, sizeof(r->name)) != 0) return -1;
            r->iat = iat + ti * 4u;
            trap = TRAP_BASE + h->import_count * TRAP_STRIDE;
            r->trap = trap;
            wr32(h->mem + r->iat, trap);
            if (h->verbosity >= 2)
                printf("MIPS-TOOL:   %08x -> %08x  %s.%s (hint %u)\n",
                       r->iat, trap, r->dll, r->name, (unsigned)r->hint);
            h->import_count++;
        }
        if (ti == MAX_IMPORTS) return -1;
    }
    if (!h->import_count) return -1;
    return 0;
}

static const import_rec_t *find_trap(const cfe_host_t *h, uint32_t pc)
{
    unsigned i;
    for (i = 0; i < h->import_count; i++)
        if (h->imports[i].trap == pc) return &h->imports[i];
    return NULL;
}

static void print_stack_words(const cfe_host_t *h, uint32_t sp)
{
    unsigned i;
    printf("MIPS-TOOL: stack");
    for (i = 0; i < 8u; i++) {
        uint32_t a = sp + i * 4u;
        if (!guest_range(h, a, 4u)) break;
        printf(" +%02x=%08x", i*4u, rd32(h->mem+a));
    }
    printf("\n");
}


static uint32_t align_up(uint32_t v, uint32_t a)
{
    return (v + a - 1u) & ~(a - 1u);
}

static uint32_t stack_arg(const cfe_host_t *h, const r3000_cpu_t *cpu, unsigned n)
{
    uint32_t a = cpu->gpr[29] + 16u + (n - 4u) * 4u;
    if (n < 4u) return cpu->gpr[4u + n];
    if (!guest_range(h, a, 4u)) return 0;
    return rd32(h->mem + a);
}

static void guest_write32(cfe_host_t *h, uint32_t a, uint32_t v)
{
    if (guest_range(h, a, 4u)) wr32(h->mem + a, v);
}

static jmp_slot_t *jmp_find(cfe_host_t *h, uint32_t guest_buf, int create)
{
    unsigned i;
    jmp_slot_t *free_slot = NULL;
    for (i = 0; i < MAX_JMP_SLOTS; i++) {
        if (h->jmps[i].valid && h->jmps[i].guest_buf == guest_buf) return &h->jmps[i];
        if (!h->jmps[i].valid && free_slot == NULL) free_slot = &h->jmps[i];
    }
    if (!create || free_slot == NULL) return NULL;
    memset(free_slot, 0, sizeof(*free_slot));
    free_slot->valid = 1;
    free_slot->guest_buf = guest_buf;
    return free_slot;
}

static void guest_write16(cfe_host_t *h, uint32_t a, uint16_t v)
{
    if (!guest_range(h, a, 2u)) return;
    h->mem[a] = (uint8_t)v;
    h->mem[a+1] = (uint8_t)(v >> 8);
}

static uint32_t host_file_alloc(cfe_host_t *h, FILE *f)
{
    unsigned i;
    for (i=0;i<MAX_FILES;i++) if (!h->files[i]) { h->files[i]=f; return FILE_HANDLE_BASE+i; }
    return 0xffffffffu;
}

static FILE *host_file(cfe_host_t *h, uint32_t handle)
{
    if (handle == 0xfffffff6u || handle == 0u) return stdin;
    if (handle == 0xfffffff5u || handle == 1u) return stdout;
    if (handle == 0xfffffff4u || handle == 2u) return stderr;
    if (handle >= FILE_HANDLE_BASE && handle < FILE_HANDLE_BASE+MAX_FILES)
        return h->files[handle-FILE_HANDLE_BASE];
    return NULL;
}

static int is_root_numeric_scratch(const char *s)
{
    const unsigned char *p=(const unsigned char *)s;
    if (!p || *p!='/') return 0;
    p++;
    if (!*p) return 0;
    while (*p) {
        if (*p<'0' || *p>'9') return 0;
        p++;
    }
    return 1;
}

static void normalize_path(char *s)
{
    char *p;

    /* Keep one internal spelling for separators.  Win32 stdio accepts '/'
       perfectly well, and preserving the drive prefix is essential for paths
       inherited from TMP/TEMP such as C:\\TEMP/cf001234. */
    for (p=s; *p; ++p) {
        if (*p=='\\') *p='/';
    }
#ifndef _WIN32
    /* The POSIX verification host cannot address Win32 drive letters.  Older
       milestones intentionally treated C:/foo as a relative foo path there.
       Never do this on the real Windows host. */
    if (s[0] && s[1]==':' && s[2]=='/') {
        memmove(s,s+3,strlen(s+3)+1u);
    }
#endif
}


static void map_host_path(char *s, size_t cap)
{
    normalize_path(s);
#ifdef _WIN32
    /* CFE still uses a Unix scratch-name convention for large compilations and
       can ask CreateFile for root names such as /2.  Keep the rule narrow:
       only /<digits> is redirected, into inherited TMP then TEMP. */
    if (is_root_numeric_scratch(s)) {
        const char *tmp=getenv("TMP");
        char out[512];
        size_t n;
        if (!tmp || !*tmp) tmp=getenv("TEMP");
        if (!tmp || !*tmp) tmp=".";
        strncpy(out,tmp,sizeof(out)-1u);
        out[sizeof(out)-1u]=0;
        normalize_path(out);
        n=strlen(out);
        while (n>0u && out[n-1u]=='/') out[--n]=0;
        if (n+strlen(s)+1u < sizeof(out)) {
            size_t copy_len;
            strcat(out,s);
            copy_len=strlen(out);
            if (copy_len>=cap) copy_len=cap-1u;
            memcpy(s,out,copy_len);
            s[copy_len]=0;
        }
    }
#else
    (void)cap;
#endif
}


static int file_exists(const char *path)
{
    FILE *f=fopen(path,"rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static void uppercase_ascii(char *s)
{
    while (*s) {
        if (*s>='a' && *s<='z') *s=(char)(*s-'a'+'A');
        s++;
    }
}

static int resolve_child_image(const cfe_host_t *h, const char *requested, char *out, size_t cap)
{
    char tmp[512], alt[512], dir[512];
    const char *base;
    size_t n;
    if (!requested || !*requested || !cap) return -1;
    strncpy(tmp,requested,sizeof(tmp)-1u);
    tmp[sizeof(tmp)-1u]=0;
    normalize_path(tmp);
#ifdef _WIN32
    /* The 1991 driver emits Unix-ish ./foo.exe / NT-ish .\\foo.exe.
       Host child lookup does not need that prefix; remove it before spawning. */
    while (tmp[0]=='.' && tmp[1]=='/') {
        memmove(tmp,tmp+2,strlen(tmp+2)+1u);
    }
#endif
    if (file_exists(tmp)) { snprintf(out,cap,"%s",tmp); return 0; }
    base=strrchr(tmp,'/'); base=base?base+1:tmp;
    strncpy(alt,base,sizeof(alt)-1u); alt[sizeof(alt)-1u]=0; uppercase_ascii(alt);
    n=strlen(alt);
    if (n<4u || strcmp(alt+n-4u,".EXE")) {
        if (n+4u<sizeof(alt)) strcat(alt,".EXE");
    }
    if (file_exists(alt)) { snprintf(out,cap,"%s",alt); return 0; }
    strncpy(dir,h->image_path,sizeof(dir)-1u); dir[sizeof(dir)-1u]=0; normalize_path(dir);
    { char *slash=strrchr(dir,'/'); if (slash) slash[1]=0; else dir[0]=0; }
    if (strlen(dir)+strlen(alt)+1u < sizeof(tmp)) {
        strcpy(tmp,dir); strcat(tmp,alt);
        if (file_exists(tmp)) { snprintf(out,cap,"%s",tmp); return 0; }
    }
    return -1;
}

static int append_shell_quoted(char *dst, size_t cap, size_t *pos, const char *s)
{
    size_t i;
#ifdef _WIN32
    /* Quote one argv element using the CommandLineToArgvW/MS CRT convention.
       This is used only to build CreateProcessA's mutable command line; no
       command shell is involved. */
    size_t slash_count = 0u;
    if (*pos + 1u >= cap) return -1;
    dst[(*pos)++] = '"';
    for (i=0u; s[i]!=0; i++) {
        if (s[i]=='\\') {
            slash_count++;
            continue;
        }
        if (s[i]=='"') {
            size_t j;
            for (j=0u; j<slash_count*2u+1u; j++) {
                if (*pos + 1u >= cap) return -1;
                dst[(*pos)++]='\\';
            }
            slash_count=0u;
            if (*pos + 1u >= cap) return -1;
            dst[(*pos)++]='"';
            continue;
        }
        while (slash_count) {
            if (*pos + 1u >= cap) return -1;
            dst[(*pos)++]='\\';
            slash_count--;
        }
        if (*pos + 1u >= cap) return -1;
        dst[(*pos)++]=s[i];
    }
    while (slash_count) {
        size_t j;
        for (j=0u; j<2u; j++) {
            if (*pos + 1u >= cap) return -1;
            dst[(*pos)++]='\\';
        }
        slash_count--;
    }
    if (*pos + 1u >= cap) return -1;
    dst[(*pos)++]='"';
    dst[*pos]=0;
#else
    if (*pos + 1u >= cap) {
        return -1;
    }
    dst[(*pos)++] = '\'';

    for (i = 0; s[i] != 0; i++) {
        if (s[i] == '\'') {
            const char q[] = "'\"'\"'";
            size_t j;

            for (j = 0; j < 5u; j++) {
                if (*pos + 1u >= cap) {
                    return -1;
                }
                dst[(*pos)++] = q[j];
            }
        } else {
            if (*pos + 1u >= cap) {
                return -1;
            }
            dst[(*pos)++] = s[i];
        }
    }

    if (*pos + 1u >= cap) {
        return -1;
    }
    dst[(*pos)++] = '\'';
    dst[*pos] = 0;
#endif
    return 0;
}

static const char *skip_first_command_token(const char *s)
{
    int quote=0;
    while (*s==' ' || *s=='\t') s++;
    if (*s=='"') { quote=1; s++; }
    while (*s) {
        if (quote) { if (*s=='"') { s++; break; } }
        else if (*s==' ' || *s=='\t') break;
        s++;
    }
    while (*s==' ' || *s=='\t') s++;
    return s;
}

static int run_child_tool(cfe_host_t *h, const char *app, const char *guest_cmd, uint32_t *exit_code)
{
    char image[512], command[4096];
    const char *rest;
    size_t pos=0;
#ifndef _WIN32
    int rc;
#endif
    command[0]=0;
    if (resolve_child_image(h,app,image,sizeof(image))!=0) {
        fprintf(stderr,"MIPS-TOOL: CreateProcess cannot resolve child '%s'\n",app?app:"");
        return -1;
    }
    if (append_shell_quoted(command,sizeof(command),&pos,h->launcher_path)!=0) return -1;
    if (h->verbosity != 1) {
        const char *vopt = (h->verbosity <= 0) ? "-q" : (h->verbosity >= 3 ? "-vv" : "-v");
        size_t n = strlen(vopt);
        if (pos + 1u + n >= sizeof(command)) return -1;
        command[pos++] = ' ';
        memcpy(command + pos, vopt, n);
        pos += n;
        command[pos] = 0;
    }
    if (pos+1u>=sizeof(command)) return -1;
    command[pos++]=' ';
    command[pos]=0;
    if (append_shell_quoted(command,sizeof(command),&pos,image)!=0) return -1;
    rest=skip_first_command_token(guest_cmd?guest_cmd:"");
    if (*rest) {
        size_t n=strlen(rest);
        if (pos+1u+n>=sizeof(command)) return -1;
        command[pos++]=' '; memcpy(command+pos,rest,n+1u);
    }
    if (h->verbosity >= 2) printf("MIPS-TOOL:   CreateProcess recursive host command: %s\n",command);
#ifdef _WIN32
    {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        DWORD code=1u;
        BOOL ok;
        memset(&si,0,sizeof(si));
        memset(&pi,0,sizeof(pi));
        si.cb=(DWORD)sizeof(si);
        if (h->verbosity >= 1) printf("MIPS-TOOL: spawn %s\n",image);
        if (h->verbosity >= 2)
            printf("MIPS-TOOL:   Win32 direct spawn (no cmd.exe): app='%s' child='%s'\n",
                   h->launcher_path,image);
        ok=CreateProcessA(h->launcher_path,command,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi);
        if (!ok) {
            fprintf(stderr,"MIPS-TOOL: CreateProcessA launcher failed: error=%lu\n",
                    (unsigned long)GetLastError());
            return -1;
        }
        WaitForSingleObject(pi.hProcess,INFINITE);
        if (!GetExitCodeProcess(pi.hProcess,&code)) code=1u;
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        *exit_code=(uint32_t)code;
    }
#else
    rc=system(command);
    if (rc<0) return -1;
    if (WIFEXITED(rc)) *exit_code=(uint32_t)WEXITSTATUS(rc);
    else if (WIFSIGNALED(rc)) *exit_code=(uint32_t)(128+WTERMSIG(rc));
    else *exit_code=1u;
#endif
    return 0;
}

static uint32_t vm_alloc(cfe_host_t *h, uint32_t requested, uint32_t size)
{
    uint32_t a, end;
    size = align_up(size ? size : 1u, 0x1000u);
    if (requested) a = requested & ~0xfffu;
    else a = align_up(h->vm_next,0x1000u);
    end = a + size;
    if (end < a || a < VM_BASE || end > VM_LIMIT || !guest_range(h,a,size)) return 0;
    memset(h->mem+a,0,size);
    if (!requested) h->vm_next=end;
    return a;
}

static void return_from_hostcall(r3000_cpu_t *cpu, uint32_t v)
{
    cpu->gpr[2]=v;
    cpu->pc=cpu->gpr[31];
    cpu->npc=cpu->pc+4u;
    cpu->next_is_delay_slot=0;
}

static int service_import(cfe_host_t *h, r3000_cpu_t *cpu, const import_rec_t *imp)
{
    uint32_t a0=cpu->gpr[4], a1=cpu->gpr[5], a2=cpu->gpr[6], a3=cpu->gpr[7];
    uint32_t ret=0;
    char path[512];
    FILE *f;
    h->hostcalls++;
    if (h->verbosity >= 3)
        printf("MIPS-TOOL: HOSTCALL #%llu %s.%s a0=%08x a1=%08x a2=%08x a3=%08x sp=%08x\n",
               (unsigned long long)h->hostcalls, imp->dll, imp->name, a0,a1,a2,a3,cpu->gpr[29]);

    if (!strcmp(imp->name,"GetCommandLine")) ret=h->command_line;
    else if (!strcmp(imp->name,"GetEnvironmentStrings")) ret=h->environment;
    else if (!strcmp(imp->name,"GetVersion")) ret=0x00000a03u; /* NT 3.10-ish: major 3 minor 10 */
    else if (!strcmp(imp->name,"BaseNullApiCall")) ret=0;
    else if (!strcmp(imp->name,"GetLastError")) ret=h->last_error;
    else if (!strcmp(imp->name,"GetStdHandle")) {
        if (a0==0xfffffff6u) ret=0; else if (a0==0xfffffff5u) ret=1; else if (a0==0xfffffff4u) ret=2; else ret=0xffffffffu;
    }
    else if (!strcmp(imp->name,"SetStdHandle")) ret=1;
    else if (!strcmp(imp->name,"GetFileType")) ret=host_file(h,a0)?2u:0u;
    else if (!strcmp(imp->name,"GetCurrentProcessId")) {
        /*
         * CC uses the PID only to make unique temporary file names.
         * Keep this personality deterministic across host platforms; this
         * reproduces the cf001234/st001234 naming used by the pipeline oracle.
         */
        ret=0x00001234u;
        if (h->verbosity >= 2) printf("MIPS-TOOL:   GetCurrentProcessId -> %u (deterministic guest PID)\n",ret);
    }
    else if (!strcmp(imp->name,"GetModuleFileName")) {
        size_t n=strlen((char*)h->mem+h->module_name);
        if (a1 && a2 && guest_range(h,a1,a2)) {
            if (n>=a2) n=a2-1u;
            memcpy(h->mem+a1,h->mem+h->module_name,n); h->mem[a1+n]=0; ret=(uint32_t)n;
        }
    }
    else if (!strcmp(imp->name,"GetStartupInfo")) {
        if (guest_range(h,a0,68u)) {
            memset(h->mem+a0,0,68u); guest_write32(h,a0,68u);
            guest_write32(h,a0+56u,0u); guest_write32(h,a0+60u,1u); guest_write32(h,a0+64u,2u);
        }
        ret=0;
    }
    else if (!strcmp(imp->name,"VirtualAlloc")) {
        ret=vm_alloc(h,a0,a1);
        if (h->verbosity >= 2) printf("MIPS-TOOL:   VirtualAlloc requested=%08x size=%08x type=%08x prot=%08x -> %08x\n",a0,a1,a2,a3,ret);
        if (!ret) h->last_error=8u;
    }
    else if (!strcmp(imp->name,"VirtualFree")) ret=1;
    else if (!strcmp(imp->name,"RtlMoveMemory")) {
        if (!guest_range(h,a0,a2)||!guest_range(h,a1,a2)) return -1;
        memmove(h->mem+a0,h->mem+a1,a2); ret=a0;
    }
    else if (!strcmp(imp->name,"RtlZeroMemory")) {
        if (!guest_range(h,a0,a1)) return -1;
        memset(h->mem+a0,0,a1); ret=a0;
    }
    else if (!strcmp(imp->name,"WriteFile")) {
        uint32_t written=0, outp=a3;
        f=host_file(h,a0);
        if (f && guest_range(h,a1,a2)) { written=(uint32_t)fwrite(h->mem+a1,1,a2,f); fflush(f); ret=1; }
        if (outp) guest_write32(h,outp,written);
    }
    else if (!strcmp(imp->name,"ReadFile")) {
        uint32_t got=0, outp=a3;
        f=host_file(h,a0);
        if (f && guest_range(h,a1,a2)) { got=(uint32_t)fread(h->mem+a1,1,a2,f); ret=1; }
        if (outp) guest_write32(h,outp,got);
    }
    else if (!strcmp(imp->name,"GetCurrentDirectory")) {
        char cwd[512]; size_t n;
        if (!getcwd(cwd,sizeof(cwd))) strcpy(cwd,".");
        n=strlen(cwd);
        if (a1 && guest_range(h,a1,a0)) {
            if (n+1u>a0) ret=(uint32_t)(n+1u);
            else { memcpy(h->mem+a1,cwd,n+1u); ret=(uint32_t)n; }
        } else ret=(uint32_t)(n+1u);
    }
    else if (!strcmp(imp->name,"SetCurrentDirectory")) {
        if (read_cstr(h,a0,path,sizeof(path))==0) { normalize_path(path); ret=(chdir(path)==0)?1u:0u; if(!ret)h->last_error=(uint32_t)errno; }
    }
    else if (!strcmp(imp->name,"GetEnvironmentVariable")) {
        char key[128]; const char *v; size_t n;
        if (read_cstr(h,a0,key,sizeof(key))!=0) ret=0;
        else { v=getenv(key); if(!v){h->last_error=203u;ret=0;} else { n=strlen(v); if(!a2||n+1u>a2)ret=(uint32_t)(n+1u); else {memcpy(h->mem+a1,v,n+1u);ret=(uint32_t)n;} } }
    }
    else if (!strcmp(imp->name,"SetEnvironmentVariable")) {
        char key[128], val[1024];
        if (read_cstr(h,a0,key,sizeof(key))!=0) ret=0u;
        else {
#ifdef _WIN32
            if (!a1) ret=SetEnvironmentVariableA(key,NULL)?1u:0u;
            else if (read_cstr(h,a1,val,sizeof(val))!=0) ret=0u;
            else ret=SetEnvironmentVariableA(key,val)?1u:0u;
#else
            /* Current recovered compiler driver only needs host environment
               inheritance on POSIX; keep mutation semantics permissive. */
            (void)val;
            ret=1u;
#endif
        }
    }
    else if (!strcmp(imp->name,"GetLogicalDrives")) ret=4u;
    else if (!strcmp(imp->name,"GetDriveType")) ret=3u;
    else if (!strcmp(imp->name,"FindFirstFile")) { h->last_error=2u; ret=0xffffffffu; }
    else if (!strcmp(imp->name,"FindClose")) ret=1u;
    else if (!strcmp(imp->name,"CreateProcess")) {
        uint32_t pip=stack_arg(h,cpu,9u);
        char appbuf[512], cmdbuf[2048];
        uint32_t ec=1u;
        appbuf[0]=0; cmdbuf[0]=0;
        if (a0) read_cstr(h,a0,appbuf,sizeof(appbuf));
        if (a1) read_cstr(h,a1,cmdbuf,sizeof(cmdbuf));
        if (h->verbosity >= 2) printf("MIPS-TOOL:   CreateProcess application='%s' command='%s'\n",appbuf,cmdbuf);
        if (!appbuf[0] && cmdbuf[0]) {
            const char *e=cmdbuf; size_t n=0;
            while (*e==' '||*e=='\t') e++;
            if (*e=='"') { e++; while(e[n]&&e[n]!='"')n++; } else while(e[n]&&e[n]!=' '&&e[n]!='\t')n++;
            if (n>=sizeof(appbuf)) n=sizeof(appbuf)-1u;
            memcpy(appbuf,e,n);
            appbuf[n]=0;
        }
        if (run_child_tool(h,appbuf,cmdbuf,&ec)!=0) { h->last_error=2u; ret=0u; }
        else {
            uint32_t id=++h->next_child_id;
            h->child_process_handle=0x70000000u+id*2u;
            h->child_thread_handle=h->child_process_handle+1u;
            h->child_exit_code=ec;
            if (pip && guest_range(h,pip,16u)) {
                guest_write32(h,pip+0u,h->child_process_handle);
                guest_write32(h,pip+4u,h->child_thread_handle);
                guest_write32(h,pip+8u,0x4000u+id);
                guest_write32(h,pip+12u,0x5000u+id);
            }
            if (h->verbosity >= 1) printf("MIPS-TOOL: child %s exited %u\n",appbuf[0]?appbuf:"<command>",ec);
            if (h->verbosity >= 2) printf("MIPS-TOOL:   child handles process=%08x thread=%08x\n",h->child_process_handle,h->child_thread_handle);
            h->last_error=0u; ret=1u;
        }
    }
    else if (!strcmp(imp->name,"WaitForSingleObject")) {
        ret=(a0==h->child_process_handle || a0==h->child_thread_handle)?0u:0xffffffffu;
        if (ret==0xffffffffu) h->last_error=6u;
    }
    else if (!strcmp(imp->name,"GetExitCodeProcess")) {
        if (a0==h->child_process_handle && a1 && guest_range(h,a1,4u)) { guest_write32(h,a1,h->child_exit_code); ret=1u; }
        else { h->last_error=6u; ret=0u; }
    }
    else if (!strcmp(imp->name,"CreateFile")) {
        uint32_t disp=stack_arg(h,cpu,4u);
        uint32_t flags=stack_arg(h,cpu,5u);
        uint32_t tmpl=stack_arg(h,cpu,6u);
        const char *mode="rb";
        char raw_path[512];
        raw_path[0]=0;
        if (read_cstr(h,a0,raw_path,sizeof(raw_path))!=0) { h->last_error=2; ret=0xffffffffu; }
        else {
            strncpy(path,raw_path,sizeof(path)-1u);
            path[sizeof(path)-1u]=0;
            map_host_path(path,sizeof(path));
            {
                const char *base=strrchr(path,'/');
                base=base?base+1:path;
                if (!strcmp(base,h->tool_name) || !strcmp(base,"cfe.exe")) {
                    strncpy(path,h->image_path,sizeof(path)-1u);
                    path[sizeof(path)-1u]=0;
                }
            }
            if (disp==2u) mode="w+b";
            else if (disp==4u) mode="a+b";
            else if (a1 & 0x40000000u) mode="r+b";
            if (h->verbosity >= 3) {
                printf("MIPS-TOOL:   CreateFile args path='%s' access=%08x share=%08x sec=%08x disp=%08x flags=%08x tmpl=%08x ra=%08x\n",
                       raw_path,a1,a2,a3,disp,flags,tmpl,cpu->gpr[31]);
                if (strcmp(raw_path,path)!=0)
                    printf("MIPS-TOOL:   CreateFile host-path '%s' -> '%s'\n",raw_path,path);
            } else if (h->verbosity >= 2 && is_root_numeric_scratch(raw_path)) {
                printf("MIPS-TOOL:   compiler scratch %s -> %s\n",raw_path,path);
            }
            errno=0;
            f=fopen(path,mode);
            if (!f && (a1&0x40000000u)) f=fopen(path,"w+b");
            if (!f) { h->last_error=(uint32_t)(errno?errno:2); ret=0xffffffffu; }
            else { h->last_error=0u; ret=host_file_alloc(h,f); }
            if (h->verbosity >= 3)
                printf("MIPS-TOOL:   CreateFile '%s' mode=%s -> %08x errno=%d\n",
                       path,mode,ret,ret==0xffffffffu?errno:0);
        }
    }
    else if (!strcmp(imp->name,"CloseHandle")) {
        if (a0>=FILE_HANDLE_BASE && a0<FILE_HANDLE_BASE+MAX_FILES && h->files[a0-FILE_HANDLE_BASE]) {
            fclose(h->files[a0-FILE_HANDLE_BASE]); h->files[a0-FILE_HANDLE_BASE]=NULL; ret=1;
        } else ret=1;
    }
    else if (!strcmp(imp->name,"SetFilePointer")) {
        long off=(long)(int32_t)a1; int whence=(a3==0u)?SEEK_SET:(a3==1u?SEEK_CUR:SEEK_END);
        f=host_file(h,a0); if (f && fseek(f,off,whence)==0) { long p=ftell(f); ret=p<0?0xffffffffu:(uint32_t)p; } else ret=0xffffffffu;
    }
    else if (!strcmp(imp->name,"SetEndOfFile")) ret=1;
    else if (!strcmp(imp->name,"FlushFileBuffers")) { f=host_file(h,a0); ret=(f && fflush(f)==0)?1u:0u; }
    else if (!strcmp(imp->name,"DeleteFile")) {
        if (read_cstr(h,a0,path,sizeof(path))==0) { normalize_path(path); ret=(remove(path)==0)?1u:0u; } else ret=0;
    }
    else if (!strcmp(imp->name,"MoveFile")) {
        char dst[512];
        if (read_cstr(h,a0,path,sizeof(path))==0 && read_cstr(h,a1,dst,sizeof(dst))==0) {
            normalize_path(path); normalize_path(dst);
            ret=(rename(path,dst)==0)?1u:0u;
            if (!ret) h->last_error=(uint32_t)errno;
            if (h->verbosity >= 2) printf("MIPS-TOOL:   MoveFile '%s' -> '%s' = %u\n",path,dst,ret);
        } else ret=0;
    }
    else if (!strcmp(imp->name,"GetFileAttributes")) {
        if (read_cstr(h,a0,path,sizeof(path))==0) {
            char resolved[512];
            normalize_path(path);
            f=fopen(path,"rb");
            if (f) { fclose(f); ret=0x80u; }
            else if (resolve_child_image(h,path,resolved,sizeof(resolved))==0) ret=0x80u;
            else { h->last_error=2u; ret=0xffffffffu; }
        } else ret=0xffffffffu;
    }
    else if (!strcmp(imp->name,"GetFileTime")) { if(a1)memset(h->mem+a1,0,8); if(a2)memset(h->mem+a2,0,8); if(a3)memset(h->mem+a3,0,8); ret=1; }
    else if (!strcmp(imp->name,"FileTimeToSystemTime")) { if (guest_range(h,a1,16u)) memset(h->mem+a1,0,16u); ret=1; }
    else if (!strcmp(imp->name,"GetSystemTime")) {
        if (guest_range(h,a0,16u)) { memset(h->mem+a0,0,16u); guest_write16(h,a0,1991); guest_write16(h,a0+2,12); guest_write16(h,a0+6,1); }
        ret=0;
    }
    else if (!strcmp(imp->name,"__C_specific_handler")) {
        fprintf(stderr,"MIPS-TOOL: __C_specific_handler reached; exception machinery not implemented\n"); return -2;
    }
    else if (!strcmp(imp->name,"setjmp")) {
        jmp_slot_t *j = jmp_find(h, a0, 1);
        unsigned i;
        if (j == NULL) { fprintf(stderr,"MIPS-TOOL: out of setjmp slots\n"); return -2; }
        for (i = 0; i < 32u; i++) j->gpr[i] = cpu->gpr[i];
        j->hi = cpu->hi;
        j->lo = cpu->lo;
        j->return_pc = cpu->gpr[31];
        ret = 0u;
    }
    else if (!strcmp(imp->name,"longjmp")) {
        jmp_slot_t *j = jmp_find(h, a0, 0);
        uint32_t value = a1 ? a1 : 1u;
        unsigned i;
        if (j == NULL) { fprintf(stderr,"MIPS-TOOL: longjmp without matching setjmp buffer %08x\n",a0); return -2; }
        for (i = 0; i < 32u; i++) cpu->gpr[i] = j->gpr[i];
        cpu->hi = j->hi;
        cpu->lo = j->lo;
        cpu->gpr[2] = value;
        cpu->gpr[0] = 0u;
        cpu->pc = j->return_pc;
        cpu->npc = j->return_pc + 4u;
        if (h->verbosity >= 2) printf("MIPS-TOOL: longjmp buffer=%08x -> %08x value=%08x\n",a0,j->return_pc,value);
        return 0;
    }
    else if (!strcmp(imp->name,"ExitProcess")) {
        h->exit_code = a0;
        if (h->verbosity >= 1) printf("MIPS-TOOL: ExitProcess(%u) after %llu instructions / %llu host calls\n",a0,(unsigned long long)cpu->instructions,(unsigned long long)h->hostcalls);
        return 1;
    }
    else {
        fprintf(stderr,"MIPS-TOOL: unsupported import %s.%s\n",imp->dll,imp->name);
        return -2;
    }
    return_from_hostcall(cpu,ret);
    return 0;
}

static int install_process_strings(cfe_host_t *h, int argc, char **argv, int tool_index)
{
    size_t pos=0; int i;
    char *cmd=(char*)h->mem+CMDLINE_ADDR;
    h->command_line=CMDLINE_ADDR; h->environment=ENV_ADDR; h->module_name=MODULE_ADDR;
    {
        uint32_t ep=ENV_ADDR;
        uint32_t env_limit=MODULE_ADDR;
        { size_t mn=strlen(h->image_path); if(mn>511u) mn=511u; memcpy(h->mem+MODULE_ADDR,h->image_path,mn); h->mem[MODULE_ADDR+mn]=0; }
#ifdef _WIN32
        {
            LPCH block=GetEnvironmentStringsA();
            if (block) {
                const char *p=block;
                while (*p) {
                    size_t n=strlen(p)+1u;
                    if ((uint64_t)ep+n+1u >= env_limit) break;
                    memcpy(h->mem+ep,p,n); ep+=(uint32_t)n; p+=n;
                }
                FreeEnvironmentStringsA(block);
            }
        }
#else
        {
            extern char **environ;
            char **e=environ;
            while (e && *e) {
                size_t n=strlen(*e)+1u;
                if ((uint64_t)ep+n+1u >= env_limit) break;
                memcpy(h->mem+ep,*e,n); ep+=(uint32_t)n; e++;
            }
        }
#endif
        h->mem[ep++]=0;
        if (h->verbosity >= 2) {
            const char *inc=getenv("INCLUDE"), *lib=getenv("LIB");
            const char *tmp=getenv("TMP"), *temp=getenv("TEMP");
            printf("MIPS-TOOL: environment inherited; INCLUDE=%s LIB=%s TMP=%s TEMP=%s\n",
                   inc?inc:"<unset>", lib?lib:"<unset>",
                   tmp?tmp:"<unset>", temp?temp:"<unset>");
        }
    }
    /* Generic transitional NT/MIPS tool runner: use the executable basename as argv[0]. */
    {
        const char *name=h->tool_name;
        size_t n=strlen(name);
        if (n>=4u && name[n-4u]=='.') n-=4u;
        if (n>=4094u) return -1;
        memcpy(cmd,name,n); pos=n;
    }
    for (i=tool_index+1;i<argc;i++) {
        const char *a=argv[i]; size_t n=strlen(a); int quote=strchr(a,' ')!=NULL;
        if (pos && pos<4094u) cmd[pos++]=' ';
        if (quote && pos<4094u) cmd[pos++]='"';
        if (pos+n+2u>=4096u) return -1;
        memcpy(cmd+pos,a,n); pos+=n;
        if (quote) cmd[pos++]='"';
    }
    cmd[pos]=0;
    if (h->verbosity >= 2) printf("MIPS-TOOL: guest command line: %s\n",cmd);
    return 0;
}

static int load_image(const char *path, cfe_host_t *h, uint32_t *entry_out)
{
    FILE *f;
    uint8_t *file = NULL;
    long flen_l;
    size_t flen;
    uint16_t machine, nscns, opthdr;
    uint32_t image_base, entry_rva;
    uint32_t shoff;
    uint32_t idata_va = 0, idata_size = 0;
    unsigned i;

    f = fopen(path, "rb");
    if (!f) { perror(path); return -1; }
    if (fseek(f, 0, SEEK_END) || (flen_l = ftell(f)) < 0 || fseek(f, 0, SEEK_SET)) {
        fclose(f); return -1;
    }
    flen = (size_t)flen_l;
    file = (uint8_t *)malloc(flen ? flen : 1u);
    if (!file) { fclose(f); return -1; }
    if (fread(file, 1, flen, f) != flen) { free(file); fclose(f); return -1; }
    fclose(f);

    if (flen < 20u) { fprintf(stderr, "MIPS-TOOL: short file\n"); free(file); return -1; }
    machine = rd16(file + 0);
    nscns = rd16(file + 2);
    opthdr = rd16(file + 16);
    if (machine != CFE_MACHINE_R3000 || !nscns || nscns > MAX_SECTIONS || opthdr < 32u) {
        fprintf(stderr, "MIPS-TOOL: not expected transitional R3000 COFF (machine=%04x sections=%u opt=%u)\n",
                machine, (unsigned)nscns, (unsigned)opthdr);
        free(file); return -1;
    }
    if (!range_ok(flen, 20u, opthdr)) { free(file); return -1; }
    if (rd16(file + 20) != 0x010bu) {
        fprintf(stderr, "MIPS-TOOL: optional magic %04x, expected 010b\n", rd16(file+20));
        free(file); return -1;
    }
    entry_rva = rd32(file + 20 + 16);
    image_base = rd32(file + 20 + 28);
    *entry_out = image_base + entry_rva;
    shoff = 20u + opthdr;
    if (!range_ok(flen, shoff, (uint32_t)nscns * 40u)) { free(file); return -1; }

    if (h->verbosity >= 2)
        printf("MIPS-TOOL: machine=%04x sections=%u image_base=%08x entry_rva=%08x entry=%08x\n",
               machine, (unsigned)nscns, image_base, entry_rva, *entry_out);

    for (i = 0; i < nscns; i++) {
        const uint8_t *s = file + shoff + i * 40u;
        char name[9];
        uint32_t rva, vsize, rawptr, va;
        memcpy(name, s, 8); name[8] = 0;
        rva = rd32(s + 12);
        vsize = rd32(s + 16);
        rawptr = rd32(s + 20);
        va = image_base + rva;
        if (h->verbosity >= 2)
            printf("MIPS-TOOL: section %-8s rva=%08x va=%08x size=%08x raw=%08x\n",
                   name, rva, va, vsize, rawptr);
        if (!guest_range(h, va, vsize)) {
            fprintf(stderr, "MIPS-TOOL: section %s lies outside 32 MiB jar\n", name);
            free(file); return -1;
        }
        memset(h->mem + va, 0, vsize);
        if (rawptr) {
            if (!range_ok(flen, rawptr, vsize)) {
                fprintf(stderr, "MIPS-TOOL: section %s raw range invalid\n", name);
                free(file); return -1;
            }
            memcpy(h->mem + va, file + rawptr, vsize);
        }
        if (!strcmp(name, ".idata")) { idata_va = va; idata_size = vsize; }
        {
            uint32_t image_end = align_up(va + vsize, 0x1000u);
            if (image_end > h->vm_next && image_end < VM_LIMIT)
                h->vm_next = image_end;
        }
    }
    free(file);
    if (!idata_va) { fprintf(stderr, "MIPS-TOOL: no .idata\n"); return -1; }
    if (patch_imports(h, idata_va, idata_size) != 0) {
        fprintf(stderr, "MIPS-TOOL: import parsing failed\n"); return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    cfe_host_t host;
    r3000_cpu_t cpu;
    r3k_bus_ops_t ops;
    uint32_t entry;
    uint64_t max_insns = 0ull; /* 0 = unlimited */
    uint64_t i;
    int tool_index=1;
    int verbosity=0;

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    if (argc >= 2 && (!strcmp(argv[1], "--version") || !strcmp(argv[1], "-V"))) {
        printf("mips-tool %s - transitional NT/MIPS R3000 user-mode runner\n", MIPS_TOOL_VERSION);
        return 0;
    }
    while (tool_index < argc && argv[tool_index][0]=='-') {
        if (!strcmp(argv[tool_index], "-q") || !strcmp(argv[tool_index], "--quiet")) {
            verbosity=0;
        } else if (!strcmp(argv[tool_index], "-v") || !strcmp(argv[tool_index], "--verbose")) {
            verbosity=2;
        } else if (!strcmp(argv[tool_index], "-vv") || !strcmp(argv[tool_index], "--trace")) {
            verbosity=3;
        } else if (!strcmp(argv[tool_index], "--limit")) {
            char *endp;
            unsigned long long v;
            if (tool_index + 1 >= argc) {
                fprintf(stderr, "mips-tool: --limit requires an instruction count (0 = unlimited)\n");
                return 2;
            }
            errno=0;
            endp=NULL;
            v=strtoull(argv[++tool_index], &endp, 0);
            if (errno || !endp || *endp) {
                fprintf(stderr, "mips-tool: invalid --limit value '%s'\n", argv[tool_index]);
                return 2;
            }
            max_insns=(uint64_t)v;
        } else if (!strncmp(argv[tool_index], "--limit=", 8)) {
            char *endp;
            unsigned long long v;
            errno=0;
            endp=NULL;
            v=strtoull(argv[tool_index]+8, &endp, 0);
            if (errno || !endp || *endp) {
                fprintf(stderr, "mips-tool: invalid --limit value '%s'\n", argv[tool_index]+8);
                return 2;
            }
            max_insns=(uint64_t)v;
        } else if (!strcmp(argv[tool_index], "--")) {
            tool_index++;
            break;
        } else {
            break;
        }
        tool_index++;
    }
    if (tool_index >= argc || !strcmp(argv[tool_index], "--help") || !strcmp(argv[tool_index], "-h")) {
        fprintf(stderr, "mips-tool %s - transitional NT/MIPS R3000 user-mode runner\n", MIPS_TOOL_VERSION);
        fprintf(stderr, "usage: %s [-q|-v|-vv] [--limit N] TOOL.EXE [tool arguments...]\n", argc > 0 ? argv[0] : "mips-tool");
        fprintf(stderr, "  -q   quiet: guest output and errors only\n");
        fprintf(stderr, "  -v   verbose: image/import/environment and child details\n");
        fprintf(stderr, "  -vv  trace: every imported API call\n");
        fprintf(stderr, "  --limit N  stop after N guest instructions; 0/default = unlimited\n");
        fprintf(stderr, "example: %s CC.EXE -v -c hello.c\n", argc > 0 ? argv[0] : "mips-tool");
        fprintf(stderr, "example: %s -v CC.EXE -v -c hello.c\n", argc > 0 ? argv[0] : "mips-tool");
        return tool_index >= argc ? 2 : 0;
    }

    memset(&host,0,sizeof(host));
    host.verbosity=verbosity;
    if (verbosity >= 1) {
        fprintf(stderr, "MIPS-TOOL %s: loading %s\n", MIPS_TOOL_VERSION, argv[tool_index]);
    }
    host.mem_size=GUEST_MEM_SIZE;
    host.mem=(uint8_t*)calloc(1,host.mem_size);
    host.vm_next=VM_BASE;
    strncpy(host.image_path,argv[tool_index],sizeof(host.image_path)-1u);
    host.image_path[sizeof(host.image_path)-1u]=0;
#ifdef _WIN32
    {
        DWORD got=GetModuleFileNameA(NULL,host.launcher_path,(DWORD)sizeof(host.launcher_path));
        if (!got || got >= (DWORD)sizeof(host.launcher_path)) {
            strncpy(host.launcher_path,argv[0],sizeof(host.launcher_path)-1u);
            host.launcher_path[sizeof(host.launcher_path)-1u]=0;
        }
    }
#else
    strncpy(host.launcher_path,argv[0],sizeof(host.launcher_path)-1u);
    host.launcher_path[sizeof(host.launcher_path)-1u]=0;
#endif
    {
        const char *base1=strrchr(argv[tool_index],'/');
        const char *base2=strrchr(argv[tool_index],'\\');
        const char *base; size_t n;
        if (base1 && base2) base=(base1>base2)?base1+1:base2+1;
        else if (base1) base=base1+1;
        else if (base2) base=base2+1;
        else base=argv[tool_index];
        n=strlen(base);
        if (n>=sizeof(host.tool_name)) n=sizeof(host.tool_name)-1u;
        memcpy(host.tool_name,base,n); host.tool_name[n]=0;
        { size_t j; for(j=0;j<n;j++) if(host.tool_name[j]>='A'&&host.tool_name[j]<='Z') host.tool_name[j]=(char)(host.tool_name[j]-'A'+'a'); }
    }
    if (!host.mem) { fprintf(stderr,"MIPS-TOOL: cannot allocate guest jar\n"); return 1; }
    if (load_image(argv[tool_index],&host,&entry)!=0) { free(host.mem); return 1; }
    if (install_process_strings(&host,argc,argv,tool_index)!=0) { fprintf(stderr,"MIPS-TOOL: command line too long\n"); free(host.mem); return 1; }

    ops.read=bus_read; ops.write=bus_write;
    r3000_init(&cpu,&ops,&host,1);
    r3000_set_flat_user_map(&cpu,1);
    r3000_set_fpu_extended(&cpu,1);
    cpu.cp0[R3K_C0_STATUS] |= R3K_SR_CU1;
    r3000_set_start(&cpu,entry);
    cpu.gpr[29]=GUEST_STACK_TOP;

    if (host.verbosity >= 2) {
        if (max_insns)
            printf("MIPS-TOOL: starting R3000 at %08x sp=%08x; instruction limit=%llu\n",
                   entry,cpu.gpr[29],(unsigned long long)max_insns);
        else
            printf("MIPS-TOOL: starting R3000 at %08x sp=%08x; instruction limit=unlimited\n",
                   entry,cpu.gpr[29]);
    }
    for (i=0; !max_insns || i<max_insns; i++) {
        const import_rec_t *imp=find_trap(&host,cpu.pc);
        if (imp) {
            int rc=service_import(&host,&cpu,imp);
            if (rc==1) {
                uint32_t guest_exit = host.exit_code;
                if (host.verbosity >= 1) {
                    printf("MIPS-TOOL: guest exit status %u\n", guest_exit);
                    printf("MIPS-TOOL: EXECUTION PROGRESS PASS\n");
                }
                free(host.mem);
                return (int)(guest_exit & 0xffu);
            }
            if (rc<0) { print_stack_words(&host,cpu.gpr[29]); r3000_dump(&cpu); free(host.mem); return 1; }
            continue;
        }
        if (cpu.pc>=0x80000000u) {
            unsigned exc=(cpu.cp0[R3K_C0_CAUSE]&R3K_CAUSE_EXCCODE)>>2;
            fprintf(stderr,"MIPS-TOOL: guest exception: pc=%08x epc=%08x cause=%08x exc=%u (%s) badvaddr=%08x after %llu host calls\n",
                    cpu.pc,cpu.cp0[R3K_C0_EPC],cpu.cp0[R3K_C0_CAUSE],exc,r3000_exception_name(exc),cpu.cp0[R3K_C0_BADVADDR],(unsigned long long)host.hostcalls);
            r3000_dump(&cpu); free(host.mem); return 1;
        }
        if (r3000_step(&cpu)!=0) { fprintf(stderr,"MIPS-TOOL: CPU stopped\n"); r3000_dump(&cpu); free(host.mem); return 1; }
    }
    fprintf(stderr,"MIPS-TOOL: instruction limit reached after %llu host calls\n",(unsigned long long)host.hostcalls);
    r3000_dump(&cpu); free(host.mem); return 1;
}
