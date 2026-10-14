#ifndef R3K_JIT_H
#define R3K_JIT_H

#include "r3000.h"

typedef struct r3k_jit r3k_jit_t;
/* Return a stable, side-effect-free RAM/ROM view, or NULL for MMIO/unmapped.
 * The returned bytes must have the same order as an instruction bus read. */
typedef const uint8_t *(*r3k_jit_code_ptr_t)(void *, uint32_t, size_t);
/* Called after EVERY instruction, including interpreter fallback. Return zero
 * to stop. If changing code, mappings or CPU control state, return zero so the
 * next run revalidates the block. Do not destroy the JIT from this callback.
 * Recursive run calls on the same JIT return zero without executing. */
typedef int (*r3k_jit_tick_t)(void *);
typedef struct r3k_jit_stats {
    uint64_t native_instructions, interpreted_instructions;
    uint64_t compiled_blocks, invalidations, allocation_failures;
} r3k_jit_stats_t;

const char *r3k_jit_backend(void); /* NULL on unsupported hosts/builds. */
r3k_jit_t *r3k_jit_create(r3000_cpu_t *cpu, r3k_jit_code_ptr_t code_ptr,
                         void *code_opaque, r3k_jit_tick_t tick, void *tick_opaque);
void r3k_jit_destroy(r3k_jit_t *jit);
/* Exact instruction budget; stops BEFORE stop_pc, if enabled. */
uint64_t r3k_jit_run(r3k_jit_t *jit, uint64_t budget,
                     int stop_pc_enabled, uint32_t stop_pc);
void r3k_jit_get_stats(const r3k_jit_t *jit, r3k_jit_stats_t *stats);

#endif
