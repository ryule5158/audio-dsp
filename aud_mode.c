/* SPDX-License-Identifier: LGPL-2.1-only */
#include "aud_mode.h"
#include <stddef.h>
#include <math.h>

#define ROOT2 1.4142135623730950488f
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

void Aud_Mode_Init(Aud_Mode *self, float sample_rate)
{
    if (self == NULL) return;
    self->freq = 500.0f;
    self->q    = 50.0f;
    self->xnm1 = self->ynm1 = self->ynm2 = 0.0f;
    self->a0 = self->a1 = self->a2 = self->d = 0.0f;
    self->lfq = self->lq = -1.0f;
    self->sr  = sample_rate;
}

void Aud_Mode_Clear(Aud_Mode *self)
{
    if (self == NULL) return;
    self->xnm1 = self->ynm1 = self->ynm2 = 0.0f;
    self->a0 = self->a1 = self->a2 = 0.0f;
    self->d   = 0.0f;
    self->lfq = -1.0f;
    self->lq  = -1.0f;
}

void Aud_Mode_SetFreq(Aud_Mode *self, float freq) { if (self) self->freq = freq; }
void Aud_Mode_SetQ(Aud_Mode *self, float q)       { if (self) self->q = q; }

float Aud_Mode_Process(Aud_Mode *self, float in)
{
    if (self == NULL) return 0.0f;
    float lfq = self->lfq, lq = self->lq;
    float xn, yn, a0 = self->a0, a1 = self->a1, a2 = self->a2, d = self->d;
    float xnm1 = self->xnm1, ynm1 = self->ynm1, ynm2 = self->ynm2;
    float kfq = self->freq, kq = self->q;

    if (lfq != kfq || lq != kq) {
        float kfreq  = kfq * (2.0f * (float)M_PI);
        float kalpha = (self->sr / kfreq);
        float kbeta  = kalpha * kalpha;
        d            = 0.5f * kalpha;
        lq           = kq;
        lfq          = kfq;
        a0           = 1.0f / (kbeta + d / kfreq);
        a1           = a0 * (1.0f - 2.0f * kbeta);
        a2           = a0 * (kbeta - d / kq);
    }
    xn = in;
    yn = a0 * xnm1 - a1 * ynm1 - a2 * ynm2;
    xnm1 = xn;
    ynm2 = ynm1;
    ynm1 = yn;
    yn   = yn * d;

    self->xnm1 = xnm1; self->ynm1 = ynm1; self->ynm2 = ynm2;
    self->lfq  = lfq;  self->lq   = lq;
    self->d    = d;     self->a0   = a0;
    self->a1   = a1;    self->a2   = a2;
    return yn;
}
