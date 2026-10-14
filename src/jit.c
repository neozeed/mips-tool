/* R3000 block translator. Native emitters are from Dynamips/Mono; see
 * jit_codegen/NOTICE.md. Guest semantics and runtime are specific to this core. */
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif
#include "jit.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#if !defined(R3K_DISABLE_JIT) && (defined(__x86_64__) || defined(_M_X64))
#define JIT_AMD64 1
#define JIT_NATIVE 1
#include "jit_codegen/amd64-codegen.h"
#elif !defined(R3K_DISABLE_JIT) && (defined(__i386__) || defined(_M_IX86))
#define JIT_NATIVE 1
#include "jit_codegen/x86-codegen.h"
#endif

#ifdef JIT_NATIVE
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif
#endif

#define BLOCK_INSNS 32u
#define CACHE_SLOTS 4096u
#define CACHE_WAYS 4u
#define EMIT_CAPACITY 8192u

typedef struct jit_block {
    uint32_t pc, pa;
    uint64_t used;
    int valid, little;
    unsigned count;
    uint8_t source[BLOCK_INSNS * 4u];
    void *code;
    size_t mapping_size;
} jit_block_t;

struct r3k_jit {
    r3000_cpu_t *cpu; /* Native prologue loads this field. */
    r3k_jit_code_ptr_t code_ptr;
    void *code_opaque;
    r3k_jit_tick_t tick;
    void *tick_opaque;
    uint64_t remaining, cache_clock;
    int stopped, stop_pc_enabled, disabled, running;
    uint32_t stop_pc;
    r3k_jit_stats_t stats;
    jit_block_t blocks[CACHE_SLOTS];
};

const char *r3k_jit_backend(void)
{
#if defined(JIT_AMD64)
    return "amd64";
#elif defined(JIT_NATIVE)
    return "x86";
#else
    return NULL;
#endif
}

static int can_enter(const r3000_cpu_t *c)
{
    uint32_t sr = c->cp0[R3K_C0_STATUS];
    return !c->halted && !c->trace && !c->load_pending &&
        !c->next_is_delay_slot && c->npc == c->pc + 4u &&
        !(sr & R3K_SR_ISC) &&
        !((sr & R3K_SR_IEC) && (sr & c->cp0[R3K_C0_CAUSE] & R3K_CAUSE_IP));
}

#ifdef JIT_NATIVE
static void free_code(jit_block_t *b)
{
    if (!b->code) return;
#ifdef _WIN32
    VirtualFree(b->code, 0, MEM_RELEASE);
#else
    munmap(b->code, b->mapping_size);
#endif
    b->code = NULL;
}

/* Never keep code writable and executable at the same time. */
static void *publish_code(const uint8_t *bytes, size_t size, size_t *mapping_size)
{
    void *p;
#ifdef _WIN32
    DWORD old;
    *mapping_size = size;
    p = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!p) return NULL;
    memcpy(p, bytes, size);
    if (!VirtualProtect(p, size, PAGE_EXECUTE_READ, &old) ||
        !FlushInstructionCache(GetCurrentProcess(), p, size)) {
        VirtualFree(p, 0, MEM_RELEASE);
        return NULL;
    }
#else
    long page = sysconf(_SC_PAGESIZE);
    if (page <= 0) return NULL;
    *mapping_size = (size + (size_t)page - 1u) / (size_t)page * (size_t)page;
    p = mmap(NULL, *mapping_size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) return NULL;
    memcpy(p, bytes, size);
    if (mprotect(p, *mapping_size, PROT_READ | PROT_EXEC) != 0) {
        munmap(p, *mapping_size);
        return NULL;
    }
    __builtin___clear_cache((char *)p, (char *)p + size);
#endif
    return p;
}

/* Called at each native instruction boundary. The translated instructions
 * cannot fault or change mappings. Device time still advances one tick at a
 * time; an IRQ raised by this tick is taken before the next instruction. */
static int retire_native(r3k_jit_t *j)
{
    r3000_cpu_t *c = j->cpu;
    c->current_pc = c->pc;
    c->current_in_delay_slot = 0;
    c->pc = c->npc;
    c->npc += 4u;
    c->gpr[0] = 0;
    c->random_index = c->random_index <= 8 ? 63 : c->random_index - 1u;
    c->cp0[R3K_C0_RANDOM] = c->random_index << 8;
    c->instructions++;
    j->stats.native_instructions++;
    j->remaining--;
    if (j->tick && !j->tick(j->tick_opaque)) j->stopped = 1;
    return !j->stopped && j->remaining &&
        !(j->stop_pc_enabled && c->pc == j->stop_pc) && can_enter(c);
}

static int supported(uint32_t w)
{
    switch (w >> 26) {
    case 0:
        switch (w & 63u) {
        case 0: case 2: case 3: case 4: case 6: case 7:
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x21: case 0x23: case 0x24: case 0x25: case 0x26:
        case 0x27: case 0x2a: case 0x2b: return 1;
        default: return 0;
        }
    case 9: case 10: case 11: case 12: case 13: case 14: case 15: return 1;
    default: return 0;
    }
}

static uint32_t decode(const uint8_t *p, int little)
{
    if (little) return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
        ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    return (uint32_t)p[3] | ((uint32_t)p[2] << 8) |
        ((uint32_t)p[1] << 16) | ((uint32_t)p[0] << 24);
}

#define GPR(r) ((int)offsetof(r3000_cpu_t, gpr) + (int)(r) * 4)
#define FIELD(f) ((int)offsetof(r3000_cpu_t, f))

/* All guest operations deliberately use 32-bit x86 encodings, including on
 * AMD64. RBX/EBX holds CPU; RBP/EBP holds JIT. Scratch EAX/ECX/EDX never live
 * across a C call. Ordinary addressing uses the host's native pointer width. */
static uint8_t *emit_guest(uint8_t *p, uint32_t w)
{
    unsigned op = w >> 26, rs = (w >> 21) & 31u, rt = (w >> 16) & 31u;
    unsigned rd = (w >> 11) & 31u, sa = (w >> 6) & 31u, fn = w & 63u;
    int32_t imm = (int16_t)w;
    int dst = GPR(op ? rt : rd), store = op ? rt != 0 : rd != 0;
    if (!op && (fn == 0x11 || fn == 0x13)) {
        dst = fn == 0x11 ? FIELD(hi) : FIELD(lo);
        store = 1;
    }
    if (!store) return p;
    if (!op) {
        if (fn == 0x10 || fn == 0x12) {
            x86_mov_reg_membase(p, X86_EAX, X86_EBX,
                               fn == 0x10 ? FIELD(hi) : FIELD(lo), 4);
        } else if (fn == 0x11 || fn == 0x13) {
            x86_mov_reg_membase(p, X86_EAX, X86_EBX, GPR(rs), 4);
        } else if (fn <= 7u) {
            x86_mov_reg_membase(p, X86_EAX, X86_EBX, GPR(rt), 4);
            if (fn & 4u) {
                x86_mov_reg_membase(p, X86_ECX, X86_EBX, GPR(rs), 4);
                x86_shift_reg(p, (fn & 3u) == 0 ? X86_SHL :
                              (fn & 3u) == 2 ? X86_SHR : X86_SAR, X86_EAX);
            } else {
                x86_shift_reg_imm(p, fn == 0 ? X86_SHL : fn == 2 ? X86_SHR : X86_SAR,
                                  X86_EAX, sa);
            }
        } else {
            x86_mov_reg_membase(p, X86_EAX, X86_EBX, GPR(rs), 4);
            x86_mov_reg_membase(p, X86_ECX, X86_EBX, GPR(rt), 4);
            switch (fn) {
            case 0x21: x86_alu_reg_reg(p, X86_ADD, X86_EAX, X86_ECX); break;
            case 0x23: x86_alu_reg_reg(p, X86_SUB, X86_EAX, X86_ECX); break;
            case 0x24: x86_alu_reg_reg(p, X86_AND, X86_EAX, X86_ECX); break;
            case 0x25: x86_alu_reg_reg(p, X86_OR, X86_EAX, X86_ECX); break;
            case 0x26: x86_alu_reg_reg(p, X86_XOR, X86_EAX, X86_ECX); break;
            case 0x27:
                x86_alu_reg_reg(p, X86_OR, X86_EAX, X86_ECX);
                x86_not_reg(p, X86_EAX); break;
            case 0x2a: case 0x2b:
                x86_alu_reg_reg(p, X86_CMP, X86_EAX, X86_ECX);
                x86_mov_reg_imm(p, X86_EAX, 0); /* MOV preserves flags. */
                x86_set_reg(p, X86_CC_LT, X86_EAX, fn == 0x2a); break;
            }
        }
    } else if (op == 15) {
        x86_mov_reg_imm(p, X86_EAX, w << 16);
    } else {
        x86_mov_reg_membase(p, X86_EAX, X86_EBX, GPR(rs), 4);
        switch (op) {
        case 9: x86_alu_reg_imm(p, X86_ADD, X86_EAX, imm); break;
        case 10: case 11:
            x86_alu_reg_imm(p, X86_CMP, X86_EAX, imm);
            x86_mov_reg_imm(p, X86_EAX, 0);
            x86_set_reg(p, X86_CC_LT, X86_EAX, op == 10); break;
        case 12: x86_alu_reg_imm(p, X86_AND, X86_EAX, w & 0xffffu); break;
        case 13: x86_alu_reg_imm(p, X86_OR, X86_EAX, w & 0xffffu); break;
        case 14: x86_alu_reg_imm(p, X86_XOR, X86_EAX, w & 0xffffu); break;
        }
    }
    x86_mov_membase_reg(p, X86_EBX, dst, X86_EAX, 4);
    return p;
}

static int compile_block(r3k_jit_t *j, jit_block_t *b)
{
    uint8_t bytes[EMIT_CAPACITY], *p = bytes, *exits[BLOCK_INSNS], *epilogue;
    unsigned i;
    /* ENDBR is a NOP on older processors and supports CET indirect entries. */
    *p++ = 0xf3; *p++ = 0x0f; *p++ = 0x1e;
#ifdef JIT_AMD64
    *p++ = 0xfa;
#else
    *p++ = 0xfb;
#endif
    x86_push_reg(p, X86_EBX);
    x86_push_reg(p, X86_EBP);
#ifdef JIT_AMD64
#ifdef _WIN32
#define JIT_ARG X86_ECX
#define JIT_STACK 40
#else
#define JIT_ARG X86_EDI
#define JIT_STACK 8
#endif
    amd64_alu_reg_imm_size(p, X86_SUB, X86_ESP, JIT_STACK, 8);
    amd64_mov_reg_reg(p, X86_EBP, JIT_ARG, 8);
    amd64_mov_reg_membase(p, AMD64_RBX, AMD64_RBP, offsetof(r3k_jit_t, cpu), 8);
#else
    x86_mov_reg_membase(p, X86_EBP, X86_ESP, 12, 4);
    x86_mov_reg_membase(p, X86_EBX, X86_EBP, offsetof(r3k_jit_t, cpu), 4);
#endif
    for (i = 0; i < b->count; i++) {
        p = emit_guest(p, decode(b->source + i * 4u, b->little));
#ifdef JIT_AMD64
        amd64_mov_reg_reg(p, JIT_ARG, X86_EBP, 8);
        amd64_mov_reg_imm_size(p, X86_EAX, (uintptr_t)retire_native, 8);
#else
        x86_push_reg(p, X86_EBP);
        x86_mov_reg_imm(p, X86_EAX, (uintptr_t)retire_native);
#endif
        x86_call_reg(p, X86_EAX);
#ifndef JIT_AMD64
        x86_alu_reg_imm(p, X86_ADD, X86_ESP, 4);
#endif
        x86_test_reg_reg(p, X86_EAX, X86_EAX);
        x86_branch32(p, X86_CC_Z, 0, 0);
        exits[i] = p - 4;
    }
    epilogue = p;
#ifdef JIT_AMD64
    amd64_alu_reg_imm_size(p, X86_ADD, X86_ESP, JIT_STACK, 8);
#endif
    x86_pop_reg(p, X86_EBP);
    x86_pop_reg(p, X86_EBX);
    x86_ret(p);
    assert((size_t)(p - bytes) <= sizeof(bytes));
    for (i = 0; i < b->count; i++) {
        int32_t rel = (int32_t)(epilogue - (exits[i] + 4));
        memcpy(exits[i], &rel, 4);
    }
    b->code = publish_code(bytes, (size_t)(p - bytes), &b->mapping_size);
    if (!b->code) {
        j->stats.allocation_failures++;
        j->disabled = 1;
        fprintf(stderr, "[jit] executable memory unavailable; using interpreter\n");
        return -1;
    }
    j->stats.compiled_blocks++;
    return 0;
}

static jit_block_t *lookup(r3k_jit_t *j)
{
    r3000_cpu_t *c = j->cpu;
    uint32_t pa, hash;
    jit_block_t *b = NULL, *set;
    const uint8_t *source;
    unsigned max_count, count, way;
    if (r3000_probe_code(c, c->pc, &pa)) return NULL;
    source = j->code_ptr(j->code_opaque, pa, 4);
    /* Fallback instructions must not evict useful native blocks. */
    if (!source || !supported(decode(source, c->little_endian))) return NULL;
    hash = (c->pc >> 2) * UINT32_C(2654435761);
    set = &j->blocks[(hash >> 22) * CACHE_WAYS];
    for (way = 0; way < CACHE_WAYS; way++) {
        jit_block_t *candidate = &set[way];
        if (candidate->valid && candidate->pc == c->pc) {
            b = candidate;
            if (b->pa == pa && b->little == c->little_endian) {
                source = j->code_ptr(j->code_opaque, pa, b->count * 4u);
                if (source && !memcmp(source, b->source, b->count * 4u)) {
                    b->used = ++j->cache_clock;
                    return b;
                }
            }
            j->stats.invalidations++;
            break;
        }
        if (!b || !candidate->valid || (b->valid && candidate->used < b->used))
            b = candidate;
    }
    free_code(b);
    memset(b, 0, sizeof(*b));
    b->pc = c->pc; b->pa = pa; b->little = c->little_endian;
    max_count = (0x1000u - (c->pc & 0xfffu)) / 4u;
    if (max_count > BLOCK_INSNS) max_count = BLOCK_INSNS;
    for (count = 0; count < max_count; count++) {
        source = j->code_ptr(j->code_opaque, pa + count * 4u, 4);
        if (!source || !supported(decode(source, c->little_endian))) break;
        memcpy(b->source + count * 4u, source, 4);
    }
    b->count = count;
    if (!count || compile_block(j, b)) return NULL;
    b->valid = 1; b->used = ++j->cache_clock;
    return b;
}

#endif /* JIT_NATIVE */

r3k_jit_t *r3k_jit_create(r3000_cpu_t *cpu, r3k_jit_code_ptr_t code_ptr,
                         void *code_opaque, r3k_jit_tick_t tick, void *tick_opaque)
{
    r3k_jit_t *j;
    if (!cpu || !code_ptr || !r3k_jit_backend()) return NULL;
    j = (r3k_jit_t *)calloc(1, sizeof(*j));
    if (!j) return NULL;
    j->cpu = cpu; j->code_ptr = code_ptr; j->code_opaque = code_opaque;
    j->tick = tick; j->tick_opaque = tick_opaque;
    return j;
}

void r3k_jit_destroy(r3k_jit_t *j)
{
    if (!j) return;
#ifdef JIT_NATIVE
    {
        unsigned i;
        for (i = 0; i < CACHE_SLOTS; i++) free_code(&j->blocks[i]);
    }
#endif
    free(j);
}

uint64_t r3k_jit_run(r3k_jit_t *j, uint64_t budget,
                     int stop_pc_enabled, uint32_t stop_pc)
{
    r3000_cpu_t *c = j->cpu;
    /* A callback must not evict a block with a native frame still on stack. */
    if (j->running) return 0;
    j->running = 1;
    j->remaining = budget; j->stopped = 0;
    j->stop_pc_enabled = stop_pc_enabled; j->stop_pc = stop_pc;
    while (j->remaining && !c->halted && !j->stopped &&
           !(stop_pc_enabled && c->pc == stop_pc)) {
#ifdef JIT_NATIVE
        jit_block_t *b;
        if (!j->disabled && can_enter(c) && (b = lookup(j)) != NULL) {
            ((void (*)(r3k_jit_t *))b->code)(j);
            continue;
        }
#else
        (void)can_enter;
#endif
        r3000_step(c);
        j->stats.interpreted_instructions++;
        j->remaining--;
        if (j->tick && !j->tick(j->tick_opaque)) j->stopped = 1;
    }
    j->running = 0;
    return budget - j->remaining;
}

void r3k_jit_get_stats(const r3k_jit_t *j, r3k_jit_stats_t *stats)
{
    *stats = j->stats;
}
