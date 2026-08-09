/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef AUD_MODE_H
#define AUD_MODE_H
#include <stdint.h>

typedef struct {
    float freq, q;
    float xnm1, ynm1, ynm2, a0, a1, a2;
    float d, lfq, lq, sr;
} Aud_Mode;

void  Aud_Mode_Init(Aud_Mode *self, float sample_rate);
float Aud_Mode_Process(Aud_Mode *self, float in);
void  Aud_Mode_Clear(Aud_Mode *self);
void  Aud_Mode_SetFreq(Aud_Mode *self, float freq);
void  Aud_Mode_SetQ(Aud_Mode *self, float q);

#endif
