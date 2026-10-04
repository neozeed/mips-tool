/* R3010 functional arithmetic and opt-in MIPS-II math extensions.
 *
 * Host IEEE binary32/binary64 plus C99 fenv; compile with -frounding-math
 * -ffp-contract=off and NEVER -ffast-math. Keep the host environment private.
 * This is not a cycle-accurate FPA: enabled FP exceptions and the R3010's
 * FPInt protocol remain unsupported. Optional extended math handles
 * subnormal arithmetic functionally without emulating the assist interrupt.
 * Unsupported exceptions are reported without writing a fabricated result.
 */
#include "r3010.h"
#include <errno.h>
#include <float.h>
#include <fenv.h>
#include <math.h>
#include <string.h>

#if FLT_RADIX != 2 || FLT_MANT_DIG != 24 || DBL_MANT_DIG != 53 || \
    FLT_MAX_EXP != 128 || DBL_MAX_EXP != 1024
#error R3010 arithmetic requires IEEE binary32 float and binary64 double
#endif
#ifdef __FAST_MATH__
#error R3010 arithmetic cannot be compiled with fast-math
#endif
#if defined(__clang__)
#pragma STDC FENV_ACCESS ON
#endif

#define FCSR_CAUSES 0x0003f000u
#define FCSR_COND   0x00800000u
#define FP_I 1u
#define FP_U 2u
#define FP_O 4u
#define FP_Z 8u
#define FP_V 16u
#define FP_E 32u

static uint64_t get_double_bits(const r3000_cpu_t *c, unsigned n)
{
    /* Register halves are independent of memory byte order (MIPS I). */
    return (uint64_t)c->fpr[n] | ((uint64_t)c->fpr[n + 1u] << 32);
}

static void put_double_bits(r3000_cpu_t *c, unsigned n, uint64_t v)
{
    c->fpr[n] = (uint32_t)v;
    c->fpr[n + 1u] = (uint32_t)(v >> 32);
}

static float single_value(uint32_t u)
{ float f; memcpy(&f, &u, sizeof(f)); return f; }
static double double_value(uint64_t u)
{ double d; memcpy(&d, &u, sizeof(d)); return d; }
static uint32_t single_bits(float f)
{ uint32_t u; memcpy(&u, &f, sizeof(u)); return u; }
static uint64_t double_bits(double d)
{ uint64_t u; memcpy(&u, &d, sizeof(u)); return u; }

/* Classification uses bits: old MIPS NaNs have the opposite signalling bit
 * to IEEE-2008 host NaNs. Never pass an old-format NaN to host arithmetic. */
static int is_nan_bits(uint64_t u, unsigned fmt)
{
    return fmt == 16u ? ((u & 0x7fffffffu) > 0x7f800000u) :
        ((u & UINT64_C(0x7fffffffffffffff)) > UINT64_C(0x7ff0000000000000));
}
static int is_snan_bits(uint64_t u, unsigned fmt)
{
    return is_nan_bits(u, fmt) && (u & (fmt == 16u ? UINT64_C(0x00400000) :
        UINT64_C(0x0008000000000000))) != 0;
}
static int is_denormal_bits(uint64_t u, unsigned fmt)
{
    uint64_t a = u & (fmt == 16u ? UINT64_C(0x7fffffff) : UINT64_C(0x7fffffffffffffff));
    return a != 0 && a < (fmt == 16u ? UINT64_C(0x00800000) : UINT64_C(0x0010000000000000));
}
static uint64_t quiet_nan(unsigned fmt)
{ return fmt == 16u ? UINT64_C(0x7fbfffff) : UINT64_C(0x7ff7ffffffffffff); }

static unsigned host_exceptions(void)
{
    int f = fetestexcept(FE_ALL_EXCEPT);
    return ((f & FE_INEXACT) ? FP_I : 0u) | ((f & FE_UNDERFLOW) ? FP_U : 0u) |
        ((f & FE_OVERFLOW) ? FP_O : 0u) | ((f & FE_DIVBYZERO) ? FP_Z : 0u) |
        ((f & FE_INVALID) ? FP_V : 0u);
}

static int finish_status(r3000_cpu_t *c, unsigned flags)
{
    unsigned enabled = (c->fcr[31] >> 7) & 31u;
    c->fcr[31] = (c->fcr[31] & ~FCSR_CAUSES) | (flags << 12);
    if ((flags & FP_E) || (flags & enabled)) return -2;
    c->fcr[31] |= (flags & 31u) << 2;
    return 0;
}

int r3010_execute(r3000_cpu_t *c, uint32_t insn)
{
    unsigned fmt = (insn >> 21) & 31u, ft = (insn >> 16) & 31u;
    unsigned fs = (insn >> 11) & 31u, fd = (insn >> 6) & 31u, fn = insn & 63u;
    unsigned outfmt = fmt, flags = 0;
    int binary = fn <= 3u, compare = fn >= 48u, condition = 0;
    int nan_a, nan_b, saved_errno;
    int fixed_round = fn >= 12u && fn <= 15u;
    int extension = fn == 4u || fixed_round;
    uint64_t a, b = 0, result = 0;
    fenv_t saved;
    static const int rounding[4] = { FE_TONEAREST, FE_TOWARDZERO, FE_UPWARD, FE_DOWNWARD };

    if (fmt != 16u && fmt != 17u && fmt != 20u) return -1;
    if (extension && !c->fpu_extended) return -1;
    if (fmt == 20u) {
        if (fn != 32u && fn != 33u) return -1;
    } else if (!binary && !extension && fn != 5u && fn != 6u && fn != 7u &&
               fn != 32u && fn != 33u && fn != 36u && !compare) return -1;
    if ((fn == 32u && fmt == 16u) || (fn == 33u && fmt == 17u)) return -1;
    /* MIPS I arithmetic names even registers in both S and D formats. */
    if ((fs & 1u) || (!compare && (fd & 1u)) ||
        ((binary || compare) && (ft & 1u)) || (compare && fd != 0u)) return -1;
    if (!binary && !compare && ft != 0u) return -1;
    if (fn == 32u) outfmt = 16u;
    if (fn == 33u) outfmt = 17u;
    if (fn == 36u || fixed_round) outfmt = 20u;
    a = fmt == 17u ? get_double_bits(c, fs) : c->fpr[fs];
    if (binary || compare) b = fmt == 17u ? get_double_bits(c, ft) : c->fpr[ft];

    if (fn == 5u || fn == 6u || fn == 7u) {
        uint64_t sign = fmt == 16u ? UINT64_C(0x80000000) : UINT64_C(0x8000000000000000);
        result = fn == 5u ? a & ~sign : fn == 7u ? a ^ sign : a;
        /* MOV is a raw transfer; legacy ABS/NEG signal Invalid on NaN. */
        if (fn != 6u && is_nan_bits(a, fmt)) {
            flags = FP_V;
            result = quiet_nan(fmt);
        }
    } else {
        nan_a = fmt != 20u && is_nan_bits(a, fmt);
        nan_b = (binary || compare) && is_nan_bits(b, fmt);
        if (!c->fpu_extended && fmt != 20u && (is_denormal_bits(a, fmt) ||
            ((binary || compare) && is_denormal_bits(b, fmt))))
            return finish_status(c, FP_E);
        if (nan_a || nan_b) {
            if (is_snan_bits(a, fmt) || ((binary || compare) && is_snan_bits(b, fmt)) ||
                (compare && (fn & 8u)) || outfmt == 20u) flags = FP_V;
            condition = (fn & 1u) != 0;
            result = outfmt == 20u ? UINT64_C(0x7fffffff) : quiet_nan(outfmt);
        } else {
            /* Volatile stores round to the declared guest precision even on
             * hosts with wider intermediate arithmetic. */
            volatile float sa, sb, sr;
            volatile double da, db, dr;
            saved_errno = errno;
            if (feholdexcept(&saved)) { errno = saved_errno; return -2; }
            /* A host may have FTZ/DAZ enabled. Start from its default IEEE
             * environment, then restore the entire saved state below. */
            if (fesetenv(FE_DFL_ENV) ||
                fesetround(rounding[fixed_round ? fn - 12u : c->fcr[31] & 3u])) {
                fesetenv(&saved); errno = saved_errno; return -2;
            }
            if (compare) {
                da = fmt == 16u ? (double)single_value((uint32_t)a) : double_value(a);
                db = fmt == 16u ? (double)single_value((uint32_t)b) : double_value(b);
                condition = ((fn & 2u) && da == db) || ((fn & 4u) && da < db);
            } else if (binary && fmt == 16u) {
                sa = single_value((uint32_t)a); sb = single_value((uint32_t)b);
                switch (fn) {
                case 0: sr = sa + sb; break;
                case 1: sr = sa - sb; break;
                case 2: sr = sa * sb; break;
                default: sr = sa / sb; break;
                }
                result = single_bits(sr);
            } else if (binary) {
                da = double_value(a); db = double_value(b);
                switch (fn) {
                case 0: dr = da + db; break;
                case 1: dr = da - db; break;
                case 2: dr = da * db; break;
                default: dr = da / db; break;
                }
                result = double_bits(dr);
            } else if (fn == 4u) {
                if (fmt == 16u) {
                    sa = single_value((uint32_t)a);
                    sr = sqrtf(sa); result = single_bits(sr);
                } else {
                    da = double_value(a);
                    dr = sqrt(da); result = double_bits(dr);
                }
            } else {
                da = fmt == 20u ? (double)(int32_t)(uint32_t)a :
                     fmt == 16u ? (double)single_value((uint32_t)a) : double_value(a);
                if (outfmt == 16u) { sr = (float)da; result = single_bits(sr); }
                else if (outfmt == 17u) result = double_bits(da);
                else {
                    dr = rint(da);
                    if (dr < -2147483648.0 || dr > 2147483647.0) {
                        flags = FP_V; result = UINT64_C(0x7fffffff);
                    } else result = (uint32_t)(int32_t)dr;
                }
            }
            flags |= host_exceptions();
            /* Invalid conversion does not additionally signal Inexact. */
            if (flags & FP_V) flags = FP_V;
            fesetenv(&saved);
            errno = saved_errno;
            if (!compare && outfmt != 20u) {
                if (is_nan_bits(result, outfmt)) result = quiet_nan(outfmt);
                if (!c->fpu_extended &&
                    (is_denormal_bits(result, outfmt) || (flags & FP_U))) flags |= FP_E;
            }
        }
    }
    if (finish_status(c, flags)) return -2;
    if (compare) c->fcr[31] = (c->fcr[31] & ~FCSR_COND) | (condition ? FCSR_COND : 0u);
    else if (outfmt == 17u) put_double_bits(c, fd, result);
    else c->fpr[fd] = (uint32_t)result;
    return 0;
}
