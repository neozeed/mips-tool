#include "r3000.h"
#include "r3010.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>

#define OP(x)   (((x) >> 26) & 0x3fu)
#define RS(x)   (((x) >> 21) & 0x1fu)
#define RT(x)   (((x) >> 16) & 0x1fu)
#define RD(x)   (((x) >> 11) & 0x1fu)
#define SA(x)   (((x) >> 6)  & 0x1fu)
#define FUN(x)  ((x) & 0x3fu)
#define IMM(x)  ((uint16_t)((x) & 0xffffu))
#define SIMM(x) ((int32_t)(int16_t)IMM(x))
#define TARGET(x) ((x) & 0x03ffffffu)

#define R3K_INDEX_P        0x80000000u
#define R3K_ENTRYLO_G      0x00000100u
#define R3K_ENTRYLO_V      0x00000200u
#define R3K_ENTRYLO_D      0x00000400u
#define R3K_ENTRYLO_N      0x00000800u
#define R3K_ENTRYLO_PFN    0xfffff000u
#define R3K_ENTRYHI_VPN    0xfffff000u
#define R3K_ENTRYHI_ASID   0x00000fc0u

static void r3k_exception(r3000_cpu_t *cpu, unsigned code, int ce,
                          int has_badvaddr, uint32_t badvaddr,
                          int use_utlb_vector);

static uint32_t add_u32(uint32_t a, int32_t b)
{
    return a + (uint32_t)b;
}

static int add_overflow(int32_t a, int32_t b, int32_t *out)
{
    int64_t v = (int64_t)a + (int64_t)b;
    *out = (int32_t)v;
    return v > INT32_MAX || v < INT32_MIN;
}

static int sub_overflow(int32_t a, int32_t b, int32_t *out)
{
    int64_t v = (int64_t)a - (int64_t)b;
    *out = (int32_t)v;
    return v > INT32_MAX || v < INT32_MIN;
}

const char *r3000_exception_name(unsigned code)
{
    static const char *const names[] = {
        "Interrupt", "TLB Modified", "TLB Load/Fetch", "TLB Store",
        "Address Error Load/Fetch", "Address Error Store", "Instruction Bus Error",
        "Data Bus Error", "Syscall", "Breakpoint", "Reserved Instruction",
        "Coprocessor Unusable", "Overflow"
    };
    if (code < sizeof(names)/sizeof(names[0]))
        return names[code];
    return "Unknown";
}

void r3000_init(r3000_cpu_t *cpu, const r3k_bus_ops_t *ops, void *opaque,
                int little_endian)
{
    memset(cpu, 0, sizeof(*cpu));
    cpu->little_endian = !!little_endian;
    cpu->bus_opaque = opaque;
    if (ops)
        cpu->bus = *ops;
    r3000_reset(cpu);
}

void r3000_reset(r3000_cpu_t *cpu)
{
    uint32_t keep_prid = cpu->cp0[R3K_C0_PRID];
    if (keep_prid == 0)
        keep_prid = 0x00000220u; /* R3000 implementation 2, revision 0x20 */
    memset(cpu->gpr, 0, sizeof(cpu->gpr));
    memset(cpu->fpr, 0, sizeof(cpu->fpr));
    memset(cpu->fcr, 0, sizeof(cpu->fcr));
    cpu->fcr[0] = R3010_FIR;
    memset(cpu->tlb, 0, sizeof(cpu->tlb));
    memset(cpu->icache, 0, sizeof(cpu->icache));
    memset(cpu->dcache, 0, sizeof(cpu->dcache));
    {
        unsigned ci;
        for (ci = 0; ci < R3K_CACHE_LINES; ci++) {
            cpu->icache[ci].tag = R3K_CACHE_INV;
            cpu->dcache[ci].tag = R3K_CACHE_INV;
        }
    }
    memset(cpu->cp0, 0, sizeof(cpu->cp0));
    cpu->hi = cpu->lo = 0;
    cpu->pc = 0xbfc00000u;
    cpu->npc = 0xbfc00004u;
    cpu->current_pc = cpu->pc;
    cpu->current_in_delay_slot = 0;
    cpu->next_is_delay_slot = 0;
    cpu->load_pending = 0;
    cpu->instructions = 0;
    cpu->random_index = 63;
    cpu->halted = 0;
    cpu->halt_code = 0;
    cpu->cp0[R3K_C0_STATUS] = R3K_SR_BEV;
    cpu->cp0[R3K_C0_PRID] = keep_prid;
    cpu->cp0[R3K_C0_RANDOM] = cpu->random_index << 8;
}

void r3000_set_start(r3000_cpu_t *cpu, uint32_t pc)
{
    cpu->pc = pc;
    cpu->npc = pc + 4;
    cpu->next_is_delay_slot = 0;
}

void r3000_set_fpu_extended(r3000_cpu_t *cpu, int enabled)
{
    cpu->fpu_extended = !!enabled;
}

void r3000_set_flat_user_map(r3000_cpu_t *cpu, int enabled)
{
    cpu->flat_user_map = !!enabled;
}

void r3000_set_prid(r3000_cpu_t *cpu, uint32_t prid)
{
    cpu->cp0[R3K_C0_PRID] = prid;
}

static void commit_pending_load(r3000_cpu_t *cpu)
{
    if (cpu->load_pending) {
        if (cpu->load_reg != 0)
            cpu->gpr[cpu->load_reg] = cpu->load_value;
        cpu->load_pending = 0;
    }
    cpu->gpr[0] = 0;
}

static void schedule_load(r3000_cpu_t *cpu, unsigned reg, uint32_t value)
{
    if (reg == 0)
        return;
    if (cpu->interlocked_loads) {
        cpu->gpr[reg] = value;
        return;
    }
    cpu->load_pending = 1;
    cpu->load_reg = reg;
    cpu->load_value = value;
}

static int phys_read(r3000_cpu_t *cpu, uint32_t paddr, unsigned size, uint32_t *v,
                     int ifetch)
{
    if (!cpu->bus.read || cpu->bus.read(cpu->bus_opaque, paddr, size, v) != 0) {
        r3k_exception(cpu, ifetch ? R3K_EXC_IBE : R3K_EXC_DBE, 0, 0, 0, 0);
        return -1;
    }
    return 0;
}

static int phys_write(r3000_cpu_t *cpu, uint32_t paddr, unsigned size, uint32_t v)
{
    if (!cpu->bus.write || cpu->bus.write(cpu->bus_opaque, paddr, size, v) != 0) {
        r3k_exception(cpu, R3K_EXC_DBE, 0, 0, 0, 0);
        return -1;
    }
    return 0;
}

/*
 * R3000 address translation.
 * kuseg (0..7fffffff) and kseg2 (c0000000..ffffffff) use the 64-entry TLB.
 * kseg0/kseg1 are direct mapped in kernel mode.
 */
static int translate_internal(r3000_cpu_t *cpu, uint32_t vaddr, int store, int ifetch,
                     uint32_t *paddr, int report_fault)
{
    uint32_t status = cpu->cp0[R3K_C0_STATUS];
    int user = !!(status & R3K_SR_KUC);
    unsigned i;
    (void)ifetch;

    if (cpu->flat_user_map && vaddr < 0x80000000u) {
        *paddr = vaddr;
        return 0;
    }

    if (user && vaddr >= 0x80000000u) {
        if (report_fault) r3k_exception(cpu, store ? R3K_EXC_ADES : R3K_EXC_ADEL,
                      0, 1, vaddr, 0);
        return -1;
    }

    if (!user && vaddr >= 0x80000000u && vaddr < 0xc0000000u) {
        *paddr = vaddr & 0x1fffffffu;
        return 0;
    }

    {
        uint32_t vpn = vaddr & R3K_ENTRYHI_VPN;
        uint32_t asid = cpu->cp0[R3K_C0_ENTRYHI] & R3K_ENTRYHI_ASID;
        int found = -1;
        for (i = 0; i < R3K_TLB_ENTRIES; i++) {
            uint32_t hi = cpu->tlb[i].entryhi;
            uint32_t lo = cpu->tlb[i].entrylo;
            if ((hi & R3K_ENTRYHI_VPN) != vpn)
                continue;
            if (!(lo & R3K_ENTRYLO_G) && ((hi & R3K_ENTRYHI_ASID) != asid))
                continue;
            found = (int)i;
            break;
        }

        if (found < 0) {
            int utlb = (vaddr < 0x80000000u);
            if (report_fault) r3k_exception(cpu, store ? R3K_EXC_TLBS : R3K_EXC_TLBL,
                          0, 1, vaddr, utlb);
            return -1;
        }

        if (!(cpu->tlb[found].entrylo & R3K_ENTRYLO_V)) {
            if (report_fault) r3k_exception(cpu, store ? R3K_EXC_TLBS : R3K_EXC_TLBL,
                          0, 1, vaddr, 0);
            return -1;
        }
        if (store && !(cpu->tlb[found].entrylo & R3K_ENTRYLO_D)) {
            if (report_fault) r3k_exception(cpu, R3K_EXC_MOD, 0, 1, vaddr, 0);
            return -1;
        }

        *paddr = (cpu->tlb[found].entrylo & R3K_ENTRYLO_PFN) | (vaddr & 0xfffu);
        return 0;
    }
}

static int translate(r3000_cpu_t *cpu, uint32_t vaddr, int store, int ifetch,
                     uint32_t *paddr)
{
    return translate_internal(cpu, vaddr, store, ifetch, paddr, 1);
}

int r3000_probe_code(r3000_cpu_t *cpu, uint32_t vaddr, uint32_t *paddr)
{
    if (vaddr & 3u) return -1;
    return translate_internal(cpu, vaddr, 0, 1, paddr, 0);
}

static unsigned cache_shift(const r3000_cpu_t *cpu, uint32_t address, unsigned size)
{
    if (size == 1u)
        return (cpu->little_endian ? (address & 3u) : ((address & 3u) ^ 3u)) * 8u;
    if (size == 2u)
        return (cpu->little_endian ? (address & 2u) : ((address & 2u) ^ 2u)) * 8u;
    return 0;
}

static r3k_cache_line_t *cache_lookup(r3000_cpu_t *cpu, uint32_t address,
                                      int invalidate, int icache, int *miss_out)
{
    r3k_cache_line_t *lines;
    r3k_cache_line_t *line;
    uint32_t mask = ~(R3K_CACHE_SIZE - 1u);
    uint32_t miss_mask = mask | R3K_CACHE_INV;
    int selected_icache;
    int miss;

    address &= ~3u;
    selected_icache = (!!icache) ^ !!(cpu->cp0[R3K_C0_STATUS] & R3K_SR_SWC);
    lines = selected_icache ? cpu->icache : cpu->dcache;
    line = &lines[(address & (R3K_CACHE_SIZE - 1u)) >> 2];

    cpu->cp0[R3K_C0_STATUS] &= ~R3K_SR_PE;
    miss = ((line->tag ^ address) & miss_mask) != 0;
    if (miss && invalidate)
        line->tag = (address & mask) | R3K_CACHE_INV;
    if (miss_out)
        *miss_out = miss;
    return line;
}

static int isolated_cache_read(r3000_cpu_t *cpu, uint32_t vaddr,
                               unsigned size, uint32_t *v)
{
    r3k_cache_line_t *line;
    uint32_t address = vaddr & ~0xe0000000u;
    unsigned shift;
    int miss;

    line = cache_lookup(cpu, address, 0, 0, &miss);
    if (miss)
        cpu->cp0[R3K_C0_STATUS] |= R3K_SR_CM;
    else
        cpu->cp0[R3K_C0_STATUS] &= ~R3K_SR_CM;
    shift = cache_shift(cpu, vaddr, size);
    if (size == 1u)
        *v = (line->data >> shift) & 0xffu;
    else if (size == 2u)
        *v = (line->data >> shift) & 0xffffu;
    else
        *v = line->data;
    return 0;
}

static int isolated_cache_write(r3000_cpu_t *cpu, uint32_t vaddr,
                                unsigned size, uint32_t v)
{
    r3k_cache_line_t *line;
    uint32_t address = vaddr & ~0xe0000000u;
    int miss;

    line = cache_lookup(cpu, address, 1, 0, &miss);
    (void)miss;
    if (size == 4u) {
        line->tag &= ~R3K_CACHE_INV;
        line->data = v;
    } else {
        /* R2000/R3000 do not support partial-word stores into isolated cache:
         * they invalidate the selected cache line. This is how old firmware
         * flushes the caches with a sequence of SB operations. */
        line->tag |= R3K_CACHE_INV;
    }
    return 0;
}

static int read8(r3000_cpu_t *cpu, uint32_t vaddr, uint32_t *v, int ifetch)
{
    uint32_t p;
    if (!ifetch && (cpu->cp0[R3K_C0_STATUS] & R3K_SR_ISC)) {
        if (cpu->cache_isolate_model)
            return isolated_cache_read(cpu, vaddr, 1, v);
        if (cpu->cache_isolate_noop) {
            *v = 0;
            return 0;
        }
    }
    if (translate(cpu, vaddr, 0, ifetch, &p) != 0)
        return -1;
    return phys_read(cpu, p, 1, v, ifetch);
}

static int read16(r3000_cpu_t *cpu, uint32_t vaddr, uint32_t *v)
{
    uint32_t p;
    if (vaddr & 1u) {
        r3k_exception(cpu, R3K_EXC_ADEL, 0, 1, vaddr, 0);
        return -1;
    }
    if (cpu->cp0[R3K_C0_STATUS] & R3K_SR_ISC) {
        if (cpu->cache_isolate_model)
            return isolated_cache_read(cpu, vaddr, 2, v);
        if (cpu->cache_isolate_noop) {
            *v = 0;
            return 0;
        }
    }
    if (translate(cpu, vaddr, 0, 0, &p) != 0)
        return -1;
    return phys_read(cpu, p, 2, v, 0);
}

static int read32(r3000_cpu_t *cpu, uint32_t vaddr, uint32_t *v, int ifetch)
{
    uint32_t p;
    if (vaddr & 3u) {
        r3k_exception(cpu, R3K_EXC_ADEL, 0, 1, vaddr, 0);
        return -1;
    }
    if (!ifetch && (cpu->cp0[R3K_C0_STATUS] & R3K_SR_ISC)) {
        if (cpu->cache_isolate_model)
            return isolated_cache_read(cpu, vaddr, 4, v);
        if (cpu->cache_isolate_noop) {
            *v = 0;
            return 0;
        }
    }
    if (translate(cpu, vaddr, 0, ifetch, &p) != 0)
        return -1;
    return phys_read(cpu, p, 4, v, ifetch);
}

static int write8(r3000_cpu_t *cpu, uint32_t vaddr, uint32_t v)
{
    uint32_t p;
    if (cpu->cp0[R3K_C0_STATUS] & R3K_SR_ISC) {
        if (cpu->cache_isolate_model)
            return isolated_cache_write(cpu, vaddr, 1, v & 0xffu);
        if (cpu->cache_isolate_noop)
            return 0;
    }
    if (translate(cpu, vaddr, 1, 0, &p) != 0)
        return -1;
    return phys_write(cpu, p, 1, v & 0xffu);
}

static int write16(r3000_cpu_t *cpu, uint32_t vaddr, uint32_t v)
{
    uint32_t p;
    if (vaddr & 1u) {
        r3k_exception(cpu, R3K_EXC_ADES, 0, 1, vaddr, 0);
        return -1;
    }
    if (cpu->cp0[R3K_C0_STATUS] & R3K_SR_ISC) {
        if (cpu->cache_isolate_model)
            return isolated_cache_write(cpu, vaddr, 2, v & 0xffffu);
        if (cpu->cache_isolate_noop)
            return 0;
    }
    if (translate(cpu, vaddr, 1, 0, &p) != 0)
        return -1;
    return phys_write(cpu, p, 2, v & 0xffffu);
}

static int write32(r3000_cpu_t *cpu, uint32_t vaddr, uint32_t v)
{
    uint32_t p;
    if (vaddr & 3u) {
        r3k_exception(cpu, R3K_EXC_ADES, 0, 1, vaddr, 0);
        return -1;
    }
    if (cpu->cp0[R3K_C0_STATUS] & R3K_SR_ISC) {
        if (cpu->cache_isolate_model)
            return isolated_cache_write(cpu, vaddr, 4, v);
        if (cpu->cache_isolate_noop)
            return 0;
    }
    if (translate(cpu, vaddr, 1, 0, &p) != 0)
        return -1;
    return phys_write(cpu, p, 4, v);
}

static void update_fault_address_regs(r3000_cpu_t *cpu, uint32_t badvaddr)
{
    cpu->cp0[R3K_C0_BADVADDR] = badvaddr;
    cpu->cp0[R3K_C0_CONTEXT] =
        (cpu->cp0[R3K_C0_CONTEXT] & 0xffe00000u) |
        ((badvaddr >> 10) & 0x001ffffcu);
}

static void update_fault_tlb_entryhi(r3000_cpu_t *cpu, uint32_t badvaddr)
{
    cpu->cp0[R3K_C0_ENTRYHI] =
        (badvaddr & R3K_ENTRYHI_VPN) |
        (cpu->cp0[R3K_C0_ENTRYHI] & R3K_ENTRYHI_ASID);
}

static void r3k_exception(r3000_cpu_t *cpu, unsigned code, int ce,
                          int has_badvaddr, uint32_t badvaddr,
                          int use_utlb_vector)
{
    uint32_t cause = cpu->cp0[R3K_C0_CAUSE];
    uint32_t status = cpu->cp0[R3K_C0_STATUS];
    uint32_t vector;

    cause &= ~(R3K_CAUSE_BD | R3K_CAUSE_CE | R3K_CAUSE_EXCCODE);
    cause |= (code << 2) & R3K_CAUSE_EXCCODE;
    cause |= ((uint32_t)ce << 28) & R3K_CAUSE_CE;
    if (cpu->current_in_delay_slot)
        cause |= R3K_CAUSE_BD;
    cpu->cp0[R3K_C0_CAUSE] = cause;

    cpu->cp0[R3K_C0_EPC] = cpu->current_in_delay_slot
                          ? cpu->current_pc - 4u
                          : cpu->current_pc;

    if (has_badvaddr) {
        update_fault_address_regs(cpu, badvaddr);
        if (code == R3K_EXC_MOD || code == R3K_EXC_TLBL || code == R3K_EXC_TLBS)
            update_fault_tlb_entryhi(cpu, badvaddr);
    }

    /* Push KU/IE three-deep stack, forcing kernel + interrupts disabled. */
    cpu->cp0[R3K_C0_STATUS] = (status & ~0x3fu) | ((status << 2) & 0x3fu);

    if (status & R3K_SR_BEV)
        vector = use_utlb_vector ? 0xbfc00100u : 0xbfc00180u;
    else
        vector = use_utlb_vector ? 0x80000000u : 0x80000080u;

    cpu->pc = vector;
    cpu->npc = vector + 4u;
    cpu->next_is_delay_slot = 0;
}

static void cp0_rfe(r3000_cpu_t *cpu)
{
    uint32_t s = cpu->cp0[R3K_C0_STATUS];
    /* Pop previous -> current and old -> previous; old pair remains. */
    cpu->cp0[R3K_C0_STATUS] = (s & ~0x0fu) | ((s >> 2) & 0x0fu);
}

static void cp0_tlbp(r3000_cpu_t *cpu)
{
    uint32_t probe = cpu->cp0[R3K_C0_ENTRYHI];
    unsigned i;
    for (i = 0; i < R3K_TLB_ENTRIES; i++) {
        uint32_t hi = cpu->tlb[i].entryhi;
        uint32_t lo = cpu->tlb[i].entrylo;
        if ((hi & R3K_ENTRYHI_VPN) != (probe & R3K_ENTRYHI_VPN))
            continue;
        if (!(lo & R3K_ENTRYLO_G) &&
            ((hi & R3K_ENTRYHI_ASID) != (probe & R3K_ENTRYHI_ASID)))
            continue;
        cpu->cp0[R3K_C0_INDEX] = i << 8;
        return;
    }
    cpu->cp0[R3K_C0_INDEX] = R3K_INDEX_P;
}

static void cp0_tlbr(r3000_cpu_t *cpu)
{
    unsigned i = (cpu->cp0[R3K_C0_INDEX] >> 8) & 0x3fu;
    cpu->cp0[R3K_C0_ENTRYHI] = cpu->tlb[i].entryhi;
    cpu->cp0[R3K_C0_ENTRYLO] = cpu->tlb[i].entrylo;
}

static void tlb_write_slot(r3000_cpu_t *cpu, unsigned i)
{
    cpu->tlb[i & 63u].entryhi = cpu->cp0[R3K_C0_ENTRYHI] & 0xffffffc0u;
    cpu->tlb[i & 63u].entrylo = cpu->cp0[R3K_C0_ENTRYLO] & 0xffffff00u;
}

static void cp0_tlbwi(r3000_cpu_t *cpu)
{
    tlb_write_slot(cpu, (cpu->cp0[R3K_C0_INDEX] >> 8) & 0x3fu);
}

static void cp0_tlbwr(r3000_cpu_t *cpu)
{
    tlb_write_slot(cpu, cpu->random_index);
}

static uint32_t cp0_read(r3000_cpu_t *cpu, unsigned r)
{
    if (r == R3K_C0_RANDOM)
        return cpu->random_index << 8;
    return cpu->cp0[r & 31u];
}

static void cp0_write(r3000_cpu_t *cpu, unsigned r, uint32_t v)
{
    switch (r & 31u) {
    case R3K_C0_INDEX:
        cpu->cp0[r] = v & (R3K_INDEX_P | 0x00003f00u);
        break;
    case R3K_C0_RANDOM:
    case R3K_C0_BADVADDR:
    case R3K_C0_PRID:
        break;
    case R3K_C0_ENTRYLO:
        cpu->cp0[r] = v & 0xffffff00u;
        break;
    case R3K_C0_CONTEXT:
        cpu->cp0[r] = (v & 0xffe00000u) | (cpu->cp0[r] & 0x001ffffcu);
        break;
    case R3K_C0_ENTRYHI:
        cpu->cp0[r] = v & 0xffffffc0u;
        break;
    case R3K_C0_CAUSE:
        /* Only software pending interrupt bits are writable. */
        cpu->cp0[r] = (cpu->cp0[r] & ~0x00000300u) | (v & 0x00000300u);
        break;
    default:
        cpu->cp0[r] = v;
        break;
    }
}

void r3000_set_interlocked_loads(r3000_cpu_t *cpu, int enabled)
{
    cpu->interlocked_loads = !!enabled;
    if (enabled)
        commit_pending_load(cpu);
}

void r3000_set_cp0_brcond0(r3000_cpu_t *cpu, int asserted)
{
    cpu->cp0_brcond0 = !!asserted;
}

void r3000_set_cache_isolate_noop(r3000_cpu_t *cpu, int enabled)
{
    cpu->cache_isolate_noop = !!enabled;
    if (enabled)
        cpu->cache_isolate_model = 0;
}

void r3000_set_cache_isolate_model(r3000_cpu_t *cpu, int enabled)
{
    cpu->cache_isolate_model = !!enabled;
    if (enabled)
        cpu->cache_isolate_noop = 0;
}

void r3000_set_hw_irq(r3000_cpu_t *cpu, unsigned line, int asserted)
{
    if (line < 6u)
        r3000_set_irq(cpu, line + 2u, asserted);
}

void r3000_set_irq(r3000_cpu_t *cpu, unsigned irq, int asserted)
{
    uint32_t bit;
    if (irq > 7)
        return;
    bit = 1u << (8u + irq);
    if (asserted)
        cpu->cp0[R3K_C0_CAUSE] |= bit;
    else
        cpu->cp0[R3K_C0_CAUSE] &= ~bit;
}

static int interrupt_pending(const r3000_cpu_t *cpu)
{
    uint32_t s = cpu->cp0[R3K_C0_STATUS];
    uint32_t c = cpu->cp0[R3K_C0_CAUSE];
    return (s & R3K_SR_IEC) && ((s & c & R3K_SR_IM) != 0);
}

static void branch_to(r3000_cpu_t *cpu, uint32_t target)
{
    cpu->npc = target;
    cpu->next_is_delay_slot = 1;
}

static int privileged_ok(r3000_cpu_t *cpu)
{
    if (cpu->cp0[R3K_C0_STATUS] & R3K_SR_KUC) {
        r3k_exception(cpu, R3K_EXC_CPU, 0, 0, 0, 0);
        return 0;
    }
    return 1;
}

static uint32_t lwl_merge(r3000_cpu_t *cpu, uint32_t old, uint32_t mem, unsigned off)
{
    if (cpu->little_endian)
        off ^= 3u;
    switch (off) {
    case 0: return mem;
    case 1: return (old & 0x000000ffu) | (mem << 8);
    case 2: return (old & 0x0000ffffu) | (mem << 16);
    default:return (old & 0x00ffffffu) | (mem << 24);
    }
}

static uint32_t lwr_merge(r3000_cpu_t *cpu, uint32_t old, uint32_t mem, unsigned off)
{
    if (cpu->little_endian)
        off ^= 3u;
    switch (off) {
    case 0: return (old & 0xffffff00u) | (mem >> 24);
    case 1: return (old & 0xffff0000u) | (mem >> 16);
    case 2: return (old & 0xff000000u) | (mem >> 8);
    default:return mem;
    }
}

static uint32_t swl_merge(r3000_cpu_t *cpu, uint32_t oldmem, uint32_t rt, unsigned off)
{
    if (cpu->little_endian)
        off ^= 3u;
    switch (off) {
    case 0: return rt;
    case 1: return (oldmem & 0xff000000u) | (rt >> 8);
    case 2: return (oldmem & 0xffff0000u) | (rt >> 16);
    default:return (oldmem & 0xffffff00u) | (rt >> 24);
    }
}

static uint32_t swr_merge(r3000_cpu_t *cpu, uint32_t oldmem, uint32_t rt, unsigned off)
{
    if (cpu->little_endian)
        off ^= 3u;
    switch (off) {
    case 0: return (oldmem & 0x00ffffffu) | (rt << 24);
    case 1: return (oldmem & 0x0000ffffu) | (rt << 16);
    case 2: return (oldmem & 0x000000ffu) | (rt << 8);
    default:return rt;
    }
}

static void trace_insn(r3000_cpu_t *cpu, uint32_t insn)
{
    if (cpu->trace)
        fprintf(stderr, "[r3k] %c pc=%08x insn=%08x%s\n",
                cpu->little_endian ? 'L' : 'B', cpu->current_pc, insn,
                cpu->current_in_delay_slot ? " [delay]" : "");
}

int r3000_step(r3000_cpu_t *cpu)
{
    uint32_t insn, curpc, rsval, rtval, merge_rt;
    unsigned op, rs, rt, rd, sa, fn;
    int was_delay;
    int old_load_pending;
    unsigned old_load_reg;
    uint32_t old_load_value;

    if (cpu->halted)
        return 1;

    cpu->current_pc = cpu->pc;
    cpu->current_in_delay_slot = cpu->next_is_delay_slot;
    curpc = cpu->pc;
    was_delay = cpu->current_in_delay_slot;

    if (interrupt_pending(cpu)) {
        commit_pending_load(cpu);
        r3k_exception(cpu, R3K_EXC_INT, 0, 0, 0, 0);
        cpu->instructions++;
        return 0;
    }

    if (read32(cpu, curpc, &insn, 1) != 0) {
        commit_pending_load(cpu);
        cpu->instructions++;
        return 0;
    }

    op = OP(insn); rs = RS(insn); rt = RT(insn); rd = RD(insn);
    sa = SA(insn); fn = FUN(insn);
    rsval = cpu->gpr[rs];
    rtval = cpu->gpr[rt];

    old_load_pending = cpu->load_pending;
    old_load_reg = cpu->load_reg;
    old_load_value = cpu->load_value;
    merge_rt = (old_load_pending && old_load_reg == rt) ? old_load_value : rtval;
    commit_pending_load(cpu);

    /* Default pipeline advance. Branches replace npc and mark the next insn. */
    cpu->pc = cpu->npc;
    cpu->npc = cpu->npc + 4u;
    cpu->next_is_delay_slot = 0;

    trace_insn(cpu, insn);

    switch (op) {
    case 0x00: /* SPECIAL */
        switch (fn) {
        case 0x00: /* SLL */
            if (rd) cpu->gpr[rd] = rtval << sa;
            break;
        case 0x02: /* SRL */
            if (rd) cpu->gpr[rd] = rtval >> sa;
            break;
        case 0x03: /* SRA */
            if (rd) cpu->gpr[rd] = (uint32_t)((int32_t)rtval >> sa);
            break;
        case 0x04: /* SLLV */
            if (rd) cpu->gpr[rd] = rtval << (rsval & 31u);
            break;
        case 0x06: /* SRLV */
            if (rd) cpu->gpr[rd] = rtval >> (rsval & 31u);
            break;
        case 0x07: /* SRAV */
            if (rd) cpu->gpr[rd] = (uint32_t)((int32_t)rtval >> (rsval & 31u));
            break;
        case 0x08: /* JR */
            branch_to(cpu, rsval);
            break;
        case 0x09: /* JALR */
            if (rd) cpu->gpr[rd] = curpc + 8u;
            branch_to(cpu, rsval);
            break;
        case 0x0c:
            r3k_exception(cpu, R3K_EXC_SYS, 0, 0, 0, 0);
            break;
        case 0x0d:
            r3k_exception(cpu, R3K_EXC_BP, 0, 0, 0, 0);
            break;
        case 0x10: if (rd) cpu->gpr[rd] = cpu->hi; break;
        case 0x11: cpu->hi = rsval; break;
        case 0x12: if (rd) cpu->gpr[rd] = cpu->lo; break;
        case 0x13: cpu->lo = rsval; break;
        case 0x18: { /* MULT */
            int64_t p = (int64_t)(int32_t)rsval * (int64_t)(int32_t)rtval;
            cpu->lo = (uint32_t)p;
            cpu->hi = (uint32_t)((uint64_t)p >> 32);
            break;
        }
        case 0x19: { /* MULTU */
            uint64_t p = (uint64_t)rsval * (uint64_t)rtval;
            cpu->lo = (uint32_t)p;
            cpu->hi = (uint32_t)(p >> 32);
            break;
        }
        case 0x1a: /* DIV */
            if (rtval != 0) {
                if (rsval == 0x80000000u && rtval == 0xffffffffu) {
                    cpu->lo = 0x80000000u; cpu->hi = 0;
                } else {
                    cpu->lo = (uint32_t)((int32_t)rsval / (int32_t)rtval);
                    cpu->hi = (uint32_t)((int32_t)rsval % (int32_t)rtval);
                }
            }
            break;
        case 0x1b: /* DIVU */
            if (rtval != 0) { cpu->lo = rsval / rtval; cpu->hi = rsval % rtval; }
            break;
        case 0x20: { /* ADD */
            int32_t out;
            if (add_overflow((int32_t)rsval, (int32_t)rtval, &out))
                r3k_exception(cpu, R3K_EXC_OV, 0, 0, 0, 0);
            else if (rd) cpu->gpr[rd] = (uint32_t)out;
            break;
        }
        case 0x21: if (rd) cpu->gpr[rd] = rsval + rtval; break;
        case 0x22: { /* SUB */
            int32_t out;
            if (sub_overflow((int32_t)rsval, (int32_t)rtval, &out))
                r3k_exception(cpu, R3K_EXC_OV, 0, 0, 0, 0);
            else if (rd) cpu->gpr[rd] = (uint32_t)out;
            break;
        }
        case 0x23: if (rd) cpu->gpr[rd] = rsval - rtval; break;
        case 0x24: if (rd) cpu->gpr[rd] = rsval & rtval; break;
        case 0x25: if (rd) cpu->gpr[rd] = rsval | rtval; break;
        case 0x26: if (rd) cpu->gpr[rd] = rsval ^ rtval; break;
        case 0x27: if (rd) cpu->gpr[rd] = ~(rsval | rtval); break;
        case 0x2a: if (rd) cpu->gpr[rd] = ((int32_t)rsval < (int32_t)rtval); break;
        case 0x2b: if (rd) cpu->gpr[rd] = (rsval < rtval); break;
        default:
            r3k_exception(cpu, R3K_EXC_RI, 0, 0, 0, 0);
            break;
        }
        break;

    case 0x01: /* REGIMM */
        switch (rt) {
        case 0x00: if ((int32_t)rsval < 0) branch_to(cpu, curpc + 4u + ((uint32_t)SIMM(insn) << 2)); else cpu->next_is_delay_slot = 1; break;
        case 0x01: if ((int32_t)rsval >= 0) branch_to(cpu, curpc + 4u + ((uint32_t)SIMM(insn) << 2)); else cpu->next_is_delay_slot = 1; break;
        case 0x10: cpu->gpr[31] = curpc + 8u; if ((int32_t)rsval < 0) branch_to(cpu, curpc + 4u + ((uint32_t)SIMM(insn) << 2)); else cpu->next_is_delay_slot = 1; break;
        case 0x11: cpu->gpr[31] = curpc + 8u; if ((int32_t)rsval >= 0) branch_to(cpu, curpc + 4u + ((uint32_t)SIMM(insn) << 2)); else cpu->next_is_delay_slot = 1; break;
        default: r3k_exception(cpu, R3K_EXC_RI, 0, 0, 0, 0); break;
        }
        break;

    case 0x02: /* J */
        branch_to(cpu, ((curpc + 4u) & 0xf0000000u) | (TARGET(insn) << 2));
        break;
    case 0x03: /* JAL */
        cpu->gpr[31] = curpc + 8u;
        branch_to(cpu, ((curpc + 4u) & 0xf0000000u) | (TARGET(insn) << 2));
        break;
    case 0x04: /* BEQ */
        if (rsval == rtval) branch_to(cpu, curpc + 4u + ((uint32_t)SIMM(insn) << 2));
        else cpu->next_is_delay_slot = 1;
        break;
    case 0x05: /* BNE */
        if (rsval != rtval) branch_to(cpu, curpc + 4u + ((uint32_t)SIMM(insn) << 2));
        else cpu->next_is_delay_slot = 1;
        break;
    case 0x06: /* BLEZ */
        if ((int32_t)rsval <= 0) branch_to(cpu, curpc + 4u + ((uint32_t)SIMM(insn) << 2));
        else cpu->next_is_delay_slot = 1;
        break;
    case 0x07: /* BGTZ */
        if ((int32_t)rsval > 0) branch_to(cpu, curpc + 4u + ((uint32_t)SIMM(insn) << 2));
        else cpu->next_is_delay_slot = 1;
        break;
    case 0x08: { /* ADDI */
        int32_t out;
        if (add_overflow((int32_t)rsval, SIMM(insn), &out))
            r3k_exception(cpu, R3K_EXC_OV, 0, 0, 0, 0);
        else if (rt) cpu->gpr[rt] = (uint32_t)out;
        break;
    }
    case 0x09: if (rt) cpu->gpr[rt] = add_u32(rsval, SIMM(insn)); break;
    case 0x0a: if (rt) cpu->gpr[rt] = ((int32_t)rsval < SIMM(insn)); break;
    case 0x0b: if (rt) cpu->gpr[rt] = (rsval < (uint32_t)SIMM(insn)); break;
    case 0x0c: if (rt) cpu->gpr[rt] = rsval & IMM(insn); break;
    case 0x0d: if (rt) cpu->gpr[rt] = rsval | IMM(insn); break;
    case 0x0e: if (rt) cpu->gpr[rt] = rsval ^ IMM(insn); break;
    case 0x0f: if (rt) cpu->gpr[rt] = (uint32_t)IMM(insn) << 16; break;

    case 0x10: /* COP0 */
        if (!privileged_ok(cpu))
            break;
        if (rs == 0x00) { /* MFC0 */
            schedule_load(cpu, rt, cp0_read(cpu, rd));
        } else if (rs == 0x04) { /* MTC0 */
            cp0_write(cpu, rd, rtval);
        } else if (rs == 0x08) { /* BC0F / BC0T */
            int take = (rt & 1u) ? cpu->cp0_brcond0 : !cpu->cp0_brcond0;
            if (take)
                branch_to(cpu, curpc + 4u + ((uint32_t)SIMM(insn) << 2));
            else
                cpu->next_is_delay_slot = 1;
        } else if (rs == 0x10) {
            switch (fn) {
            case 0x01: cp0_tlbr(cpu); break;
            case 0x02: cp0_tlbwi(cpu); break;
            case 0x06: cp0_tlbwr(cpu); break;
            case 0x08: cp0_tlbp(cpu); break;
            case 0x10: cp0_rfe(cpu); break;
            default: r3k_exception(cpu, R3K_EXC_RI, 0, 0, 0, 0); break;
            }
        } else {
            r3k_exception(cpu, R3K_EXC_RI, 0, 0, 0, 0);
        }
        break;

    case 0x11: /* COP1 / functional R3010 arithmetic and transfers */
        if (!(cpu->cp0[R3K_C0_STATUS] & R3K_SR_CU1)) {
            r3k_exception(cpu, R3K_EXC_CPU, 1, 0, 0, 0);
            break;
        }
        if ((rs == 0x00 || rs == 0x02 || rs == 0x04 || rs == 0x06) &&
            (insn & 0x7ffu)) {
            r3k_exception(cpu, R3K_EXC_RI, 0, 0, 0, 0);
        } else if (rs == 0x00) { /* MFC1 */
            schedule_load(cpu, rt, cpu->fpr[rd]);
        } else if (rs == 0x02) { /* CFC1 */
            schedule_load(cpu, rt, rd == 0 ? R3010_FIR : rd == 31 ?
                          cpu->fcr[31] & R3010_FCSR_MASK : 0u);
        } else if (rs == 0x04) { /* MTC1 */
            cpu->fpr[rd] = rtval;
        } else if (rs == 0x06) { /* CTC1 */
            if (rd == 31) cpu->fcr[31] = rtval & R3010_FCSR_MASK;
        } else if (rs == 0x08 && rt > 1u) {
            /* Branch-likely and multiple condition codes are not MIPS I. */
            r3k_exception(cpu, R3K_EXC_RI, 0, 0, 0, 0);
        } else if (rs == 0x08) { /* BC1F / BC1T, condition bit in FCR31 */
            int cond = !!(cpu->fcr[31] & 0x00800000u);
            int take = (rt & 1u) ? cond : !cond;
            if (take) branch_to(cpu, curpc + 4u + ((uint32_t)SIMM(insn) << 2));
            else cpu->next_is_delay_slot = 1;
        } else if (rs >= 0x10) {
            int fp_rc = r3010_execute(cpu, insn);
            if (fp_rc)
                fprintf(stderr, "[r3010] %s at pc=%08x insn=%08x fcsr=%08x fs=%08x:%08x ft=%08x:%08x mode=%s\n",
                        fp_rc == -2 ? "unsupported FP interrupt/assist" : "unsupported FP encoding",
                        curpc, insn, cpu->fcr[31], cpu->fpr[(rd + 1u) & 31u], cpu->fpr[rd],
                        cpu->fpr[(rt + 1u) & 31u], cpu->fpr[rt],
                        cpu->fpu_extended ? "extended" : "r3010");
            if (fp_rc) r3k_exception(cpu, R3K_EXC_RI, 0, 0, 0, 0);
        } else {
            r3k_exception(cpu, R3K_EXC_RI, 0, 0, 0, 0);
        }
        break;
    case 0x12:
        r3k_exception(cpu, R3K_EXC_CPU, 2, 0, 0, 0);
        break;
    case 0x13:
        r3k_exception(cpu, R3K_EXC_CPU, 3, 0, 0, 0);
        break;

    case 0x20: { uint32_t v; if (read8(cpu, add_u32(rsval,SIMM(insn)), &v, 0)==0) schedule_load(cpu,rt,(uint32_t)(int32_t)(int8_t)v); break; }
    case 0x21: { uint32_t v; if (read16(cpu,add_u32(rsval,SIMM(insn)),&v)==0) schedule_load(cpu,rt,(uint32_t)(int32_t)(int16_t)v); break; }
    case 0x22: { /* LWL */
        uint32_t a = add_u32(rsval,SIMM(insn)), m;
        if (read32(cpu, a & ~3u, &m, 0)==0)
            schedule_load(cpu, rt, lwl_merge(cpu, merge_rt, m, a & 3u));
        break;
    }
    case 0x23: { uint32_t v; if (read32(cpu,add_u32(rsval,SIMM(insn)),&v,0)==0) schedule_load(cpu,rt,v); break; }
    case 0x24: { uint32_t v; if (read8(cpu, add_u32(rsval,SIMM(insn)), &v, 0)==0) schedule_load(cpu,rt,v & 0xffu); break; }
    case 0x25: { uint32_t v; if (read16(cpu,add_u32(rsval,SIMM(insn)),&v)==0) schedule_load(cpu,rt,v & 0xffffu); break; }
    case 0x26: { /* LWR */
        uint32_t a = add_u32(rsval,SIMM(insn)), m;
        if (read32(cpu, a & ~3u, &m, 0)==0)
            schedule_load(cpu, rt, lwr_merge(cpu, merge_rt, m, a & 3u));
        break;
    }
    case 0x31: { /* LWC1 */
        uint32_t v;
        if (!(cpu->cp0[R3K_C0_STATUS] & R3K_SR_CU1)) { r3k_exception(cpu,R3K_EXC_CPU,1,0,0,0); break; }
        if (read32(cpu, add_u32(rsval,SIMM(insn)), &v, 0) == 0) cpu->fpr[rt] = v;
        break;
    }
    case 0x32: /* LWC2 */ r3k_exception(cpu, R3K_EXC_CPU, 2, 0, 0, 0); break;
    case 0x33: /* LWC3 */ r3k_exception(cpu, R3K_EXC_CPU, 3, 0, 0, 0); break;
    case 0x28: write8(cpu, add_u32(rsval,SIMM(insn)), rtval); break;
    case 0x29: write16(cpu, add_u32(rsval,SIMM(insn)), rtval); break;
    case 0x2a: { /* SWL */
        uint32_t a = add_u32(rsval,SIMM(insn)), m, p;
        if (read32(cpu, a & ~3u, &m, 0)==0) {
            p = swl_merge(cpu,m,rtval,a&3u);
            write32(cpu,a&~3u,p);
        }
        break;
    }
    case 0x2b: write32(cpu, add_u32(rsval,SIMM(insn)), rtval); break;
    case 0x39: /* SWC1 */
        if (!(cpu->cp0[R3K_C0_STATUS] & R3K_SR_CU1)) r3k_exception(cpu,R3K_EXC_CPU,1,0,0,0);
        else write32(cpu, add_u32(rsval,SIMM(insn)), cpu->fpr[rt]);
        break;
    case 0x3a: /* SWC2 */ r3k_exception(cpu, R3K_EXC_CPU, 2, 0, 0, 0); break;
    case 0x3b: /* SWC3 */ r3k_exception(cpu, R3K_EXC_CPU, 3, 0, 0, 0); break;
    case 0x2e: { /* SWR */
        uint32_t a = add_u32(rsval,SIMM(insn)), m, p;
        if (read32(cpu, a & ~3u, &m, 0)==0) {
            p = swr_merge(cpu,m,rtval,a&3u);
            write32(cpu,a&~3u,p);
        }
        break;
    }

    default:
        r3k_exception(cpu, R3K_EXC_RI, 0, 0, 0, 0);
        break;
    }

    cpu->gpr[0] = 0;

    /* R3000 Random cycles through 63..8 and never selects wired entries 0..7. */
    if (cpu->random_index <= 8)
        cpu->random_index = 63;
    else
        cpu->random_index--;
    cpu->cp0[R3K_C0_RANDOM] = cpu->random_index << 8;

    cpu->instructions++;
    (void)was_delay;
    return 0;
}

int r3000_run(r3000_cpu_t *cpu, uint64_t max_instructions)
{
    uint64_t start = cpu->instructions;
    while (!cpu->halted && (cpu->instructions - start) < max_instructions)
        r3000_step(cpu);
    if (!cpu->halted)
        return -1;
    return cpu->halt_code;
}

void r3000_dump(const r3000_cpu_t *cpu)
{
    static const char *const n[32] = {
        "zero","at","v0","v1","a0","a1","a2","a3",
        "t0","t1","t2","t3","t4","t5","t6","t7",
        "s0","s1","s2","s3","s4","s5","s6","s7",
        "t8","t9","k0","k1","gp","sp","fp","ra"
    };
    unsigned i;
    fprintf(stderr,"\nR3000 state after %llu instructions\n",
            (unsigned long long)cpu->instructions);
    fprintf(stderr," PC=%08x NPC=%08x HI=%08x LO=%08x\n",
            cpu->pc,cpu->npc,cpu->hi,cpu->lo);
    for (i=0;i<32;i++) {
        fprintf(stderr," %-4s=%08x%s",n[i],cpu->gpr[i],((i&3u)==3u)?"\n":" ");
    }
    fprintf(stderr," C0 Status=%08x Cause=%08x EPC=%08x BadVAddr=%08x\n",
            cpu->cp0[R3K_C0_STATUS],cpu->cp0[R3K_C0_CAUSE],
            cpu->cp0[R3K_C0_EPC],cpu->cp0[R3K_C0_BADVADDR]);
    fprintf(stderr," C0 EntryHi=%08x EntryLo=%08x Index=%08x Random=%08x Context=%08x\n",
            cpu->cp0[R3K_C0_ENTRYHI],cpu->cp0[R3K_C0_ENTRYLO],
            cpu->cp0[R3K_C0_INDEX],cpu->cp0[R3K_C0_RANDOM],
            cpu->cp0[R3K_C0_CONTEXT]);
}
