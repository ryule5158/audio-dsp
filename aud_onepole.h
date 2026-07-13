/**
 * @file    aud_onepole.h
 * @brief   One-pole lowpass / highpass filter.
 *          Ported from DaisySP Filters/onepole (MIT)
 *          Original by Emilie Gillet (pichenettes/stmlib)
 *
 *          Uses tanf() for frequency warping compensation.
 *          Cutoff valid from 0 to ~0.497 * sample_rate.
 */
#ifndef AUD_ONEPOLE_H
#define AUD_ONEPOLE_H
#include "aud_dsp.h"
#include <stdint.h>

typedef enum {
    AUD_ONEPOLE_LOW_PASS,
    AUD_ONEPOLE_HIGH_PASS
} Aud_OnePole_Mode;

typedef struct {
    float g;      /* tan(PI * freq_norm) */
    float gi;     /* 1 / (1 + g) */
    float state;  /* filter state */
    Aud_OnePole_Mode mode;
} Aud_OnePole;

static inline void Aud_OnePole_Init(Aud_OnePole *f)
{
    if (f == NULL) return;
    f->state = 0.0f;
    f->mode  = AUD_ONEPOLE_LOW_PASS;
}

static inline void Aud_OnePole_Reset(Aud_OnePole *f)
{
    if (f == NULL) return;
    f->state = 0.0f;
}

/**
 * @brief  Set cutoff (normalized freq, 0 ~ 0.497).
 *         freq_norm = cutoff_hz / sample_rate
 */
static inline void Aud_OnePole_SetFrequency(Aud_OnePole *f, float freq_norm)
{
    if (f == NULL) return;
    freq_norm = (freq_norm < 0.497f) ? freq_norm : 0.497f;
    f->g  = tanf(AUD_PI * freq_norm);
    f->gi = 1.0f / (1.0f + f->g);
}

static inline void Aud_OnePole_SetMode(Aud_OnePole *f, Aud_OnePole_Mode m)
{
    if (f == NULL) return;
    f->mode = m;
}

static inline float Aud_OnePole_Process(Aud_OnePole *f, float in)
{
    if (f == NULL) return in;
    float lp = (f->g * in + f->state) * f->gi;
    f->state = f->g * (in - lp) + lp;
    switch (f->mode) {
        case AUD_ONEPOLE_LOW_PASS:  return lp;
        case AUD_ONEPOLE_HIGH_PASS: return in - lp;
    }
    return 0.0f;
}

/** Process a block in-place. */
static inline void Aud_OnePole_ProcessBlock(Aud_OnePole *f,
                                            float *in_out, uint32_t size)
{
    if (f == NULL || in_out == NULL) return;
    for (uint32_t i = 0; i < size; i++) {
        in_out[i] = Aud_OnePole_Process(f, in_out[i]);
    }
}

#endif /* AUD_ONEPOLE_H */
