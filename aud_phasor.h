/**
 * @file    aud_phasor.h
 * @brief   Normalized phase accumulator (DDS core). Output ramps from 0 to 1.
 *          Ported from DaisySP Control/phasor (MIT)
 */
#ifndef AUD_PHASOR_H
#define AUD_PHASOR_H
#include <stdint.h>

typedef struct {
    float freq;         /* Current frequency (Hz) */
    float sample_rate;  /* Sample rate (Hz) */
    float inc;          /* Phase increment per sample (radians) */
    float phs;          /* Current phase (radians, 0 ~ 2*PI) */
} Aud_Phasor;

/**
 * @brief  Initialize phasor.
 * @param  p              Instance
 * @param  sample_rate    Sample rate in Hz
 * @param  freq           Frequency in Hz
 * @param  initial_phase  Starting phase in radians
 */
void Aud_Phasor_Init(Aud_Phasor *p, float sample_rate,
                     float freq, float initial_phase);

/** Set frequency in Hz. */
void Aud_Phasor_SetFreq(Aud_Phasor *p, float freq);

/**
 * @brief  Advance one sample and return normalized phase [0, 1).
 *         0.0=0°, 0.25=90°, 0.5=180°, 0.75=270°, etc.
 */
float Aud_Phasor_Process(Aud_Phasor *p);

/** Get current frequency. */
static inline float Aud_Phasor_GetFreq(const Aud_Phasor *p) {
    return p->freq;
}

#endif /* AUD_PHASOR_H */
