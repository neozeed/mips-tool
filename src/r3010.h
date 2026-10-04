#ifndef R3010_H
#define R3010_H

#include "r3000.h"

#define R3010_FIR 0x00000300u /* R3010-compatible FPA, implementation 3. */
#define R3010_FCSR_MASK 0x0083ffffu

/* Functional arithmetic subset. 0 = retired, -1 = unsupported encoding,
 * -2 = an operand/exception needs the not-yet-modeled R3010 FP interrupt.
 * Neither failure writes a destination register or the condition bit. */
int r3010_execute(r3000_cpu_t *cpu, uint32_t insn);

#endif
