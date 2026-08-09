/* SPDX-License-Identifier: LGPL-2.1-only */
#include "aud_balance.h"
#include <stddef.h>
#include "aud_dsp.h"
#include <math.h>

void Aud_Balance_Init(Aud_Balance *self, float sample_rate)
{
    if (self == NULL) return;
    self->sample_rate = sample_rate;
    self->ihp         = 10.0f;
    float b           = 2.0f - cosf(self->ihp * (AUD_TWOPI / sample_rate));
    self->c2          = b - sqrtf(b * b - 1.0f);
    self->c1          = 1.0f - self->c2;
    self->prvq = self->prvr = self->prva = 0.0f;
}

void Aud_Balance_SetCutoff(Aud_Balance *self, float cutoff)
{
    if (self == NULL) return;
    self->ihp = cutoff;
}

float Aud_Balance_Process(Aud_Balance *self, float sig, float comp)
{
    if (self == NULL) return sig;
    float c1 = self->c1, c2 = self->c2;
    float q  = self->prvq, r = self->prvr;
    float a, diff, out;

    q = c1 * sig * sig + c2 * q;
    r = c1 * comp * comp + c2 * r;
    self->prvq = q;
    self->prvr = r;

    if (q != 0.0f) a = sqrtf(r / q);
    else           a = sqrtf(r);

    diff = a - self->prva;
    if (diff != 0.0f) out = sig * self->prva;
    else              out = sig * a;

    self->prva = a;
    return out;
}
