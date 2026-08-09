/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef AUD_COMPRESSOR_H
#define AUD_COMPRESSOR_H
#include <stdint.h>
#include <stdbool.h>
#include "aud_dsp.h"

typedef struct {
    float ratio, thresh, atk, rel;
    float makeup_gain, gain;
    float slope_rec, gain_rec;
    float atk_slo2, ratio_mul, atk_slo, rel_slo;
    int   sample_rate;
    float sample_rate_inv2, sample_rate_inv;
    bool  makeup_auto;
} Aud_Compressor;

void  Aud_Compressor_Init(Aud_Compressor *self, float sample_rate);
float Aud_Compressor_Process(Aud_Compressor *self, float in);
float Aud_Compressor_ProcessSidechain(Aud_Compressor *self, float in, float key);
float Aud_Compressor_Apply(Aud_Compressor *self, float in);
void  Aud_Compressor_ProcessBlock(Aud_Compressor *self, float *in, float *out, uint32_t size);
void  Aud_Compressor_SetRatio(Aud_Compressor *self, float ratio);
void  Aud_Compressor_SetThreshold(Aud_Compressor *self, float threshold);
void  Aud_Compressor_SetAttack(Aud_Compressor *self, float attack);
void  Aud_Compressor_SetRelease(Aud_Compressor *self, float release);
void  Aud_Compressor_SetMakeup(Aud_Compressor *self, float gain);
void  Aud_Compressor_AutoMakeup(Aud_Compressor *self, bool enable);
float Aud_Compressor_GetGain(Aud_Compressor *self);

#endif
