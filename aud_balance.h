/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef AUD_BALANCE_H
#define AUD_BALANCE_H
#include <stdint.h>

typedef struct {
    float sample_rate, ihp, c2, c1, prvq, prvr, prva;
} Aud_Balance;

void  Aud_Balance_Init(Aud_Balance *self, float sample_rate);
float Aud_Balance_Process(Aud_Balance *self, float sig, float comp);
void  Aud_Balance_SetCutoff(Aud_Balance *self, float cutoff);

#endif
