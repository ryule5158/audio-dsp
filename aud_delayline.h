/**
 * @file    aud_delayline.h
 * @brief   Fractional delay line with linear / Hermite interpolation.
 *          Ported from DaisySP Utility/delayline (MIT)
 *
 *          Usage (static allocation):
 *            float buf[1024];
 *            Aud_DelayLine dl;
 *            Aud_DelayLine_Init(&dl, buf, 1024);
 */
#ifndef AUD_DELAYLINE_H
#define AUD_DELAYLINE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    float  *line;        /* Ring buffer (caller-allocated) */
    uint32_t max_size;   /* Buffer capacity */
    uint32_t write_ptr;  /* Current write position */
    float   delay;       /* Current delay in samples (can be fractional) */
    float   frac;        /* Fractional part of delay [0, 1) */
} Aud_DelayLine;

/* ---- Lifecycle ---- */
static inline void Aud_DelayLine_Init(Aud_DelayLine *dl,
                                      float *buffer,
                                      uint32_t size)
{
    if (dl == NULL || buffer == NULL || size == 0u) return;
    dl->line      = buffer;
    dl->max_size  = size;
    dl->write_ptr = 0;
    dl->delay     = 1.0f;
    dl->frac      = 0.0f;
    for (uint32_t i = 0; i < size; i++) dl->line[i] = 0.0f;
}

static inline void Aud_DelayLine_Reset(Aud_DelayLine *dl)
{
    if (dl == NULL || dl->line == NULL) return;
    for (uint32_t i = 0; i < dl->max_size; i++) dl->line[i] = 0.0f;
    dl->write_ptr = 0;
    dl->delay     = 1.0f;
    dl->frac      = 0.0f;
}

/* ---- Set delay (integer or fractional samples) ---- */
static inline void Aud_DelayLine_SetDelay(Aud_DelayLine *dl, float delay)
{
    if (dl == NULL || dl->max_size == 0u) return;
    if (delay < 0.0f) delay = 0.0f;
    int32_t id = (int32_t)delay;
    dl->frac   = delay - (float)id;
    dl->delay  = (float)((uint32_t)id < dl->max_size
                         ? id
                         : (int32_t)(dl->max_size - 1u));
}

/* ---- Write one sample ---- */
static inline void Aud_DelayLine_Write(Aud_DelayLine *dl, float sample)
{
    if (dl == NULL || dl->line == NULL || dl->max_size == 0u) return;
    dl->line[dl->write_ptr] = sample;
    dl->write_ptr = (dl->write_ptr == 0u)
                    ? (dl->max_size - 1u)
                    : (dl->write_ptr - 1u);
}

/* ---- Read at current delay setting (linear interpolation) ---- */
static inline float Aud_DelayLine_Read(const Aud_DelayLine *dl)
{
    if (dl == NULL || dl->line == NULL || dl->max_size == 0u) return 0.0f;
    uint32_t idx1 = (dl->write_ptr + (uint32_t)dl->delay) % dl->max_size;
    uint32_t idx2 = (idx1 + 1u) % dl->max_size;
    float a = dl->line[idx1];
    float b = dl->line[idx2];
    return a + (b - a) * dl->frac;
}

/* ---- Read at arbitrary fractional delay (linear interpolation) ---- */
static inline float Aud_DelayLine_ReadAt(const Aud_DelayLine *dl, float d)
{
    if (dl == NULL || dl->line == NULL || dl->max_size == 0u) return 0.0f;
    if (d < 0.0f) d = 0.0f;
    int32_t di = (int32_t)d;
    float   df = d - (float)di;
    uint32_t i1 = (dl->write_ptr + (uint32_t)di) % dl->max_size;
    uint32_t i2 = (i1 + 1u) % dl->max_size;
    float a = dl->line[i1];
    float b = dl->line[i2];
    return a + (b - a) * df;
}

/* ---- Read at arbitrary delay (Hermite cubic interpolation) ---- */
static inline float Aud_DelayLine_ReadHermite(const Aud_DelayLine *dl, float d)
{
    if (dl == NULL || dl->line == NULL || dl->max_size == 0u) return 0.0f;
    if (d < 0.0f) d = 0.0f;
    int32_t di = (int32_t)d;
    float   df = d - (float)di;
    uint32_t t  = dl->write_ptr + (uint32_t)di + dl->max_size;
    float   xm1 = dl->line[(t - 1u) % dl->max_size];
    float   x0  = dl->line[t % dl->max_size];
    float   x1  = dl->line[(t + 1u) % dl->max_size];
    float   x2  = dl->line[(t + 2u) % dl->max_size];
    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b_neg = w + a;
    return (((a * df) - b_neg) * df + c) * df + x0;
}

/* ---- Allpass read/write step (for fractional-delay based effects) ---- */
static inline float Aud_DelayLine_Allpass(Aud_DelayLine *dl,
                                          float sample,
                                          uint32_t delay,
                                          float coeff)
{
    if (dl == NULL || dl->line == NULL || dl->max_size == 0u) return 0.0f;
    if (delay >= dl->max_size) delay = dl->max_size - 1u;

    float read  = dl->line[(dl->write_ptr + delay) % dl->max_size];
    float write = sample + coeff * read;
    Aud_DelayLine_Write(dl, write);
    return read - write * coeff;
}

#endif /* AUD_DELAYLINE_H */
