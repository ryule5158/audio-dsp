/* SPDX-License-Identifier: LGPL-2.1-only */
#include <stddef.h>
#include "aud_tone.h"
#include <math.h>

/* ======================== Tone ======================== */
void Aud_Tone_Init(Aud_Tone *t, float sample_rate)
{
    if (t == NULL) return;
    t->sample_rate = sample_rate;
    t->c1 = 0.0f; t->c2 = 0.0f;
    t->y1_1 = 0.0f;
    Aud_Tone_SetFreq(t, 1000.0f);
}

void Aud_Tone_SetFreq(Aud_Tone *t, float freq)
{
    if (t == NULL) return;
    float b = 2.0f - cosf(AUD_TWOPI * freq / t->sample_rate);
    t->c2 = b - sqrtf(b * b - 1.0f);
    t->c1 = 1.0f - t->c2;
}

float Aud_Tone_Process(Aud_Tone *t, float in)
{
    if (t == NULL) return in;
    float y = t->c1 * in + t->c2 * t->y1_1;
    t->y1_1 = y;
    return y;
}

/* ======================== ATone ======================== */
void Aud_ATone_Init(Aud_ATone *t, float sample_rate)
{
    if (t == NULL) return;
    t->sample_rate = sample_rate;
    t->c1 = 0.0f; t->c2 = 0.0f;
    t->y1_1 = 0.0f;
    Aud_ATone_SetFreq(t, 1000.0f);
}

void Aud_ATone_SetFreq(Aud_ATone *t, float freq)
{
    if (t == NULL) return;
    float b = 2.0f - cosf(AUD_TWOPI * freq / t->sample_rate);
    t->c2 = b - sqrtf(b * b - 1.0f);
    t->c1 = 1.0f - t->c2;
}

float Aud_ATone_Process(Aud_ATone *t, float in)
{
    if (t == NULL) return in;
    float y = t->c1 * in + t->c2 * t->y1_1;
    t->y1_1 = y;
    return in - y;  /* highpass = input - lowpass */
}
