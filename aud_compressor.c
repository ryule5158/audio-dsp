/* SPDX-License-Identifier: LGPL-2.1-only */
#include "aud_compressor.h"
#include <stddef.h>
#include <math.h>

#ifndef max
#define max(a,b) ((a < b) ? b : a)
#endif
#ifndef min
#define min(a,b) ((a < b) ? a : b)
#endif

static void Aud_Compressor_RecalcRatio(Aud_Compressor *self)
{
    self->ratio_mul = ((1.0f - self->atk_slo2) * ((1.0f / self->ratio) - 1.0f));
}

static void Aud_Compressor_RecalcAttack(Aud_Compressor *self)
{
    self->atk_slo  = expf(-(self->sample_rate_inv / self->atk));
    self->atk_slo2 = expf(-(self->sample_rate_inv2 / self->atk));
    Aud_Compressor_RecalcRatio(self);
}

static void Aud_Compressor_RecalcRelease(Aud_Compressor *self)
{
    self->rel_slo = expf(-(self->sample_rate_inv / self->rel));
}

static void Aud_Compressor_RecalcMakeup(Aud_Compressor *self)
{
    if (self->makeup_auto)
        self->makeup_gain = fabsf(self->thresh - self->thresh / self->ratio) * 0.5f;
}

void Aud_Compressor_Init(Aud_Compressor *self, float sample_rate)
{
    if (self == NULL) return;
    self->sample_rate     = (int)min(192000, max(1, (int)sample_rate));
    self->sample_rate_inv = 1.0f / sample_rate;
    self->sample_rate_inv2= 2.0f / sample_rate;
    self->gain_rec  = 0.1f;
    self->slope_rec = 0.1f;

    Aud_Compressor_SetRatio(self, 2.0f);
    Aud_Compressor_SetAttack(self, 0.1f);
    Aud_Compressor_SetRelease(self, 0.1f);
    Aud_Compressor_SetThreshold(self, -12.0f);
    Aud_Compressor_AutoMakeup(self, true);
}

float Aud_Compressor_Process(Aud_Compressor *self, float in)
{
    if (self == NULL) return in;
    float inAbs   = fabsf(in);
    float cur_slo = ((self->slope_rec > inAbs) ? self->rel_slo : self->atk_slo);
    self->slope_rec = ((self->slope_rec * cur_slo) + ((1.0f - cur_slo) * inAbs));
    self->gain_rec  = ((self->atk_slo2 * self->gain_rec)
                     + (self->ratio_mul
                        * aud_fmax((20.0f * aud_fastlog10f(self->slope_rec))
                                    - self->thresh, 0.0f)));
    self->gain = aud_pow10f(0.05f * (self->gain_rec + self->makeup_gain));
    return self->gain * in;
}

float Aud_Compressor_ProcessSidechain(Aud_Compressor *self, float in, float key)
{
    Aud_Compressor_Process(self, key);
    return Aud_Compressor_Apply(self, in);
}

float Aud_Compressor_Apply(Aud_Compressor *self, float in)
{
    return (self != NULL) ? self->gain * in : in;
}

void Aud_Compressor_ProcessBlock(Aud_Compressor *self, float *in, float *out, uint32_t size)
{
    if (self == NULL || in == NULL || out == NULL) return;
    for (uint32_t i = 0; i < size; i++) {
        Aud_Compressor_Process(self, in[i]);
        out[i] = Aud_Compressor_Apply(self, in[i]);
    }
}

void Aud_Compressor_SetRatio(Aud_Compressor *self, float ratio)
    { if (self) { self->ratio = ratio; Aud_Compressor_RecalcRatio(self); } }
void Aud_Compressor_SetThreshold(Aud_Compressor *self, float t)
    { if (self) { self->thresh = t; Aud_Compressor_RecalcMakeup(self); } }
void Aud_Compressor_SetAttack(Aud_Compressor *self, float a)
    { if (self) { self->atk = a; Aud_Compressor_RecalcAttack(self); } }
void Aud_Compressor_SetRelease(Aud_Compressor *self, float r)
    { if (self) { self->rel = r; Aud_Compressor_RecalcRelease(self); } }
void Aud_Compressor_SetMakeup(Aud_Compressor *self, float g)
    { if (self) self->makeup_gain = g; }
void Aud_Compressor_AutoMakeup(Aud_Compressor *self, bool enable)
    { if (self) { self->makeup_auto = enable; self->makeup_gain = 0.0f; Aud_Compressor_RecalcMakeup(self); } }
float Aud_Compressor_GetGain(Aud_Compressor *self)
    { return (self != NULL) ? aud_fastlog10f(self->gain) * 20.0f : 0.0f; }
