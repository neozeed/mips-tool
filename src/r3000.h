#ifndef R3000_H
#define R3000_H

#include <stdint.h>
#include <stddef.h>

#define R3K_TLB_ENTRIES 64
#define R3K_CACHE_SIZE   0x10000u
#define R3K_CACHE_LINES  (R3K_CACHE_SIZE / 4u)

/* CP0 register numbers used by R2000/R3000. */
enum {
    R3K_C0_INDEX    = 0,
    R3K_C0_RANDOM   = 1,
    R3K_C0_ENTRYLO  = 2,
    R3K_C0_CONTEXT  = 4,
    R3K_C0_BADVADDR = 8,
    R3K_C0_ENTRYHI  = 10,
    R3K_C0_STATUS   = 12,
    R3K_C0_CAUSE    = 13,
    R3K_C0_EPC      = 14,
    R3K_C0_PRID     = 15
};

/* R3000 Status register bits. */
#define R3K_SR_IEC       0x00000001u
#define R3K_SR_KUC       0x00000002u
#define R3K_SR_IEP       0x00000004u
#define R3K_SR_KUP       0x00000008u
#define R3K_SR_IEO       0x00000010u
#define R3K_SR_KUO       0x00000020u
#define R3K_SR_IM        0x0000ff00u
#define R3K_SR_ISC       0x00010000u
#define R3K_SR_SWC       0x00020000u
#define R3K_SR_PZ        0x00040000u
#define R3K_SR_CM        0x00080000u
#define R3K_SR_PE        0x00100000u
#define R3K_SR_TS        0x00200000u
#define R3K_SR_BEV       0x00400000u
#define R3K_SR_RE        0x02000000u
#define R3K_SR_CU0       0x10000000u
#define R3K_SR_CU1       0x20000000u
#define R3K_SR_CU2       0x40000000u
#define R3K_SR_CU3       0x80000000u

/* R3000 Cause register bits. */
#define R3K_CAUSE_EXCCODE 0x0000007cu
#define R3K_CAUSE_IP      0x0000ff00u
#define R3K_CAUSE_CE      0x30000000u
#define R3K_CAUSE_BD      0x80000000u

/* Exception codes. */
enum {
    R3K_EXC_INT  = 0,
    R3K_EXC_MOD  = 1,
    R3K_EXC_TLBL = 2,
    R3K_EXC_TLBS = 3,
    R3K_EXC_ADEL = 4,
    R3K_EXC_ADES = 5,
    R3K_EXC_IBE  = 6,
    R3K_EXC_DBE  = 7,
    R3K_EXC_SYS  = 8,
    R3K_EXC_BP   = 9,
    R3K_EXC_RI   = 10,
    R3K_EXC_CPU  = 11,
    R3K_EXC_OV   = 12
};

typedef struct r3k_tlb_entry {
    uint32_t entryhi;
    uint32_t entrylo;
} r3k_tlb_entry_t;

typedef struct r3k_cache_line {
    uint32_t tag;
    uint32_t data;
} r3k_cache_line_t;

#define R3K_CACHE_INV 0x00000001u

typedef struct r3k_bus_ops {
    int (*read)(void *opaque, uint32_t paddr, unsigned size, uint32_t *value);
    int (*write)(void *opaque, uint32_t paddr, unsigned size, uint32_t value);
} r3k_bus_ops_t;

typedef struct r3000_cpu {
    uint32_t gpr[32];
    uint32_t hi, lo;
    uint32_t fpr[32];
    uint32_t fcr[32];
    uint32_t pc, npc;
    uint32_t current_pc;
    uint32_t cp0[32];
    r3k_tlb_entry_t tlb[R3K_TLB_ENTRIES];
    /*
     * R3000 cache compatibility arrays. Normal cached execution remains
     * coherent/direct; these lines become architecturally visible only while
     * Status.IsC is set. This is deliberate for the 1991 NT/Jazz target.
     */
    r3k_cache_line_t icache[R3K_CACHE_LINES];
    r3k_cache_line_t dcache[R3K_CACHE_LINES];

    int little_endian;
    int current_in_delay_slot;
    int next_is_delay_slot;

    int load_pending;
    unsigned load_reg;
    uint32_t load_value;

    uint64_t instructions;
    uint32_t random_index;
    int halted;
    int halt_code;
    int trace;
    int interlocked_loads;
    int cp0_brcond0;
    int cache_isolate_noop;
    int cache_isolate_model;
    int fpu_extended; /* Optional MIPS-II math opcodes + gradual underflow. */
    int flat_user_map; /* Personality mode: identity-map 0..7fffffff without TLB. */

    void *bus_opaque;
    r3k_bus_ops_t bus;
} r3000_cpu_t;

void r3000_init(r3000_cpu_t *cpu, const r3k_bus_ops_t *ops, void *opaque,
                int little_endian);
void r3000_reset(r3000_cpu_t *cpu);
void r3000_set_start(r3000_cpu_t *cpu, uint32_t pc);
void r3000_set_fpu_extended(r3000_cpu_t *cpu, int enabled);
void r3000_set_flat_user_map(r3000_cpu_t *cpu, int enabled);
void r3000_set_prid(r3000_cpu_t *cpu, uint32_t prid);
/* Side-effect-free instruction address translation for JIT lookup. */
int r3000_probe_code(r3000_cpu_t *cpu, uint32_t vaddr, uint32_t *paddr);
int r3000_step(r3000_cpu_t *cpu);
int r3000_run(r3000_cpu_t *cpu, uint64_t max_instructions);
void r3000_set_irq(r3000_cpu_t *cpu, unsigned irq, int asserted);
void r3000_set_hw_irq(r3000_cpu_t *cpu, unsigned line, int asserted);
void r3000_set_interlocked_loads(r3000_cpu_t *cpu, int enabled);
void r3000_set_cp0_brcond0(r3000_cpu_t *cpu, int asserted);
void r3000_set_cache_isolate_noop(r3000_cpu_t *cpu, int enabled);
void r3000_set_cache_isolate_model(r3000_cpu_t *cpu, int enabled);
void r3000_dump(const r3000_cpu_t *cpu);
const char *r3000_exception_name(unsigned code);

#endif
