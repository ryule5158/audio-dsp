/**
 * @file    aud_dsp.h
 * @brief   Audio DSP utility functions — ported from DaisySP dsp.h
 * @note    Fast math, one-pole smoothing, BLEP helpers, soft clipping etc.
 *          Original: Electrosmith DaisySP (MIT)
 */
#ifndef AUD_DSP_H
#define AUD_DSP_H

#include <stdint.h>
#include <math.h>

#define AUD_PI      3.14159265358979323846f
#define AUD_TWOPI   (2.0f * AUD_PI)
#define AUD_HALFPI  (AUD_PI * 0.5f)

#define AUD_MIN(a,b)  ((a) < (b) ? (a) : (b))
#define AUD_MAX(a,b)  ((a) > (b) ? (a) : (b))
#define AUD_CLAMP(x,mn,mx) (AUD_MIN(AUD_MAX(x, mn), mx))

/* ====================================================================
 *  Fast floating-point min/max (ARM FPU vmaxnm / vminnm)
 * ==================================================================== */
/* VMAXNM/VMINNM are available on the H743 FPv5-D16 FPU, but not on every
 * target that defines __arm__ (notably the i.MX6ULL Cortex-A7/VFPv4).
 * Guard the inline assembly by the FPv5 feature macro so the portable
 * implementation is selected for ARMv7-A and other ARM FPUs. */
#if defined(__arm__) && defined(__ARM_FPV5__)
static inline float aud_fmax(float a, float b) {
    float r;
    __asm__ volatile("vmaxnm.f32 %[d], %[n], %[m]" : [d] "=t"(r) : [n] "t"(a), [m] "t"(b) :);
    return r;
}
static inline float aud_fmin(float a, float b) {
    float r;
    __asm__ volatile("vminnm.f32 %[d], %[n], %[m]" : [d] "=t"(r) : [n] "t"(a), [m] "t"(b) :);
    return r;
}
#else
static inline float aud_fmax(float a, float b) { return (a > b) ? a : b; }
static inline float aud_fmin(float a, float b) { return (a < b) ? a : b; }
#endif

static inline float aud_fclamp(float x, float mn, float mx) {
    return aud_fmin(aud_fmax(x, mn), mx);
}

/* ====================================================================
 *  One-pole lowpass: out += coeff * (in - out)
 *  coeff = 1.0 / (time_seconds * sample_rate)
 * ==================================================================== */
static inline void aud_fonepole(float *out, float in, float coeff) {
    *out += coeff * (in - *out);
}

/* ====================================================================
 *  Fast power-of-10:  pow10f(x) ≈ expf(2.302585... * x)
 * ==================================================================== */
static inline float aud_pow10f(float x) {
    return expf(2.302585092994046f * x);
}

/* ====================================================================
 *  Fast log2 (polynomial approx, ~25% faster than log2f)
 *  From ARM community / CMSIS-DSP
 * ==================================================================== */
static inline float aud_fastlog2f(float f) {
    float frac;
    int   exp;
    frac = frexpf(fabsf(f), &exp);
    f    = 1.23149591368684f;
    f   *= frac;
    f   += -4.11852516267426f;
    f   *= frac;
    f   += 6.02197014179219f;
    f   *= frac;
    f   += -3.13396450166353f;
    f   += (float)exp;
    return f;
}

static inline float aud_fastlog10f(float f) {
    return aud_fastlog2f(f) * 0.3010299956639812f;
}

/* ====================================================================
 *  MIDI note → frequency
 * ==================================================================== */
static inline float aud_mtof(float midi_note) {
    return powf(2.0f, (midi_note - 69.0f) / 12.0f) * 440.0f;
}

/* ====================================================================
 *  Fractional part of float (faster than fmodf(x, 1.0f))
 * ==================================================================== */
static inline float aud_fastmod1f(float x) {
    return x - (float)((int)x);
}

/* ====================================================================
 *  3-point median filter
 * ==================================================================== */
static inline float aud_median3(float a, float b, float c) {
    return (b < a)
        ? ((b < c) ? ((c < a) ? c : a) : b)
        : ((a < c) ? ((c < b) ? c : b) : a);
}

/* ====================================================================
 *  polyBLEP correction samples
 *  Ported from pichenettes/eurorack/plaits
 * ==================================================================== */
static inline float aud_this_blep(float t) {
    return 0.5f * t * t;
}
static inline float aud_next_blep(float t) {
    t = 1.0f - t;
    return -0.5f * t * t;
}
static inline float aud_next_integrated_blep(float t) {
    const float t1 = 0.5f * t;
    const float t2 = t1 * t1;
    const float t4 = t2 * t2;
    return 0.1875f - t1 + 1.5f * t2 - t4;
}
static inline float aud_this_integrated_blep(float t) {
    return aud_next_integrated_blep(1.0f - t);
}

/* ====================================================================
 *  Soft clip / soft limit
 * ==================================================================== */
static inline float aud_soft_limit(float x) {
    return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
}
static inline float aud_soft_clip(float x) {
    if      (x < -3.0f) return -1.0f;
    else if (x >  3.0f) return  1.0f;
    else                return aud_soft_limit(x);
}

/* ====================================================================
 *  Float validity check (debug aid)
 * ==================================================================== */
static inline void aud_test_float(float *x, float fallback) {
    if (!isnormal(*x) && *x != 0.0f) {
        *x = fallback;
    }
}

/* ====================================================================
 *  Linear / exp / log mapping (0..1 → min..max)
 * ==================================================================== */
typedef enum {
    AUD_MAP_LINEAR,
    AUD_MAP_EXP,
    AUD_MAP_LOG
} Aud_MapCurve;

static inline float aud_fmap(float in, float mn, float mx, Aud_MapCurve curve) {
    switch (curve) {
    case AUD_MAP_EXP:
        return aud_fclamp(mn + (in * in) * (mx - mn), mn, mx);
    case AUD_MAP_LOG: {
        if (mn <= 0.0f) return mn;
        const float a = 1.0f / log10f(mx / mn);
        return aud_fclamp(mn * powf(10.0f, in / a), mn, mx);
    }
    case AUD_MAP_LINEAR:
    default:
        return aud_fclamp(mn + in * (mx - mn), mn, mx);
    }
}

/* ====================================================================
 *  Power-of-2 helpers
 * ==================================================================== */
static inline int aud_is_power2(uint32_t x) {
    return ((x - 1u) & x) == 0u;
}
static inline uint32_t aud_next_power2(uint32_t x) {
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

#endif /* AUD_DSP_H */
