/* SPDX-License-Identifier: LGPL-2.1-only */
/**
 * @file    aud_moogladder.h
 * @brief   Moog ladder filter — classic 4-pole lowpass with resonance.
 *          Ported from DaisySP Filters/moogladder (MIT)
 */
#ifndef AUD_MOOGLADDER_H
#define AUD_MOOGLADDER_H

typedef struct {
    float sample_rate;
    float cutoff;
    float res;
    float stage[4];    /* 4 filter stages */
    float p0;          /* feedback delay */
} Aud_MoogLadder;

void Aud_MoogLadder_Init(Aud_MoogLadder *ml, float sample_rate);
void Aud_MoogLadder_SetFreq(Aud_MoogLadder *ml, float freq);
void Aud_MoogLadder_SetRes(Aud_MoogLadder *ml, float res);
float Aud_MoogLadder_Process(Aud_MoogLadder *ml, float in);

#endif /* AUD_MOOGLADDER_H */
