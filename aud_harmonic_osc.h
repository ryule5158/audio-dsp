#ifndef AUD_HARMONIC_OSC_H
#define AUD_HARMONIC_OSC_H

/**
 * @file    aud_harmonic_osc.h
 * @brief   Chebyshev harmonic oscillator ported from DaisySP (MIT).
 * @note    Original class: DaisySP/Synthesis/HarmonicOscillator.
 */

#include <stdbool.h>
#include <stdint.h>
#include "aud_dsp.h"

#define AUD_HARMONIC_OSC_NUM_HARMONICS 16

typedef struct {
    float sample_rate_;
    float phase_;
    float frequency_;
    float amplitude_[AUD_HARMONIC_OSC_NUM_HARMONICS];
    float newamplitude_[AUD_HARMONIC_OSC_NUM_HARMONICS];
    bool  recalc_;
    int   first_harmonic_index_;
} Aud_HarmonicOscillator;

/* Backward-compatible alias for the initial C port's misspelled type name. */
typedef Aud_HarmonicOscillator Aud_HarmonicOscillatorillator;

void  Aud_HarmonicOscillator_Init(Aud_HarmonicOscillator *self, float sample_rate);
float Aud_HarmonicOscillator_Process(Aud_HarmonicOscillator *self);
void  Aud_HarmonicOscillator_SetFreq(Aud_HarmonicOscillator *self, float freq);
void  Aud_HarmonicOscillator_SetFirstHarmIdx(Aud_HarmonicOscillator *self, int idx);
void  Aud_HarmonicOscillator_SetAmplitudes(Aud_HarmonicOscillator *self, const float *amplitudes);
void  Aud_HarmonicOscillator_SetSingleAmp(Aud_HarmonicOscillator *self, float amp, int idx);

void  Aud_HarmonicOscillatorillator_Init(Aud_HarmonicOscillatorillator *self, float sample_rate);
float Aud_HarmonicOscillatorillator_Process(Aud_HarmonicOscillatorillator *self, float in);
void  Aud_HarmonicOscillatorillator_SetFreq(Aud_HarmonicOscillatorillator *self, float freq);

#endif /* AUD_HARMONIC_OSC_H */
