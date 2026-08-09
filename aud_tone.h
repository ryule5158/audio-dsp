/* SPDX-License-Identifier: LGPL-2.1-only */
/**
 * @file    aud_tone.h
 * @brief   First-order tone filter (simple bass/treble control).
 *          Ported from DaisySP Filters/tone (MIT)
 */
#ifndef AUD_TONE_H
#define AUD_TONE_H
#include "aud_dsp.h"

typedef struct {
    float c1, c2;    /* coefficients */
    float y1_1;      /* state: delayed output */
    float sample_rate;
} Aud_Tone;

void Aud_Tone_Init(Aud_Tone *t, float sample_rate);
void Aud_Tone_SetFreq(Aud_Tone *t, float freq);
float Aud_Tone_Process(Aud_Tone *t, float in);

/* ---- ATone (allpass-based highpass tone) ---- */
typedef struct {
    float c1, c2;
    float y1_1;
    float sample_rate;
} Aud_ATone;

void Aud_ATone_Init(Aud_ATone *t, float sample_rate);
void Aud_ATone_SetFreq(Aud_ATone *t, float freq);
float Aud_ATone_Process(Aud_ATone *t, float in);

#endif /* AUD_TONE_H */
