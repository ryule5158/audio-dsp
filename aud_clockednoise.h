/**
 * @file    aud_clockednoise.h
 * @brief   Clocked noise — noise S/H at a target frequency with BLEP smoothing.
 *          Ported from DaisySP Noise/clockednoise (MIT). Original by Emilie Gillet, 2016.
 */
#ifndef AUD_CLOCKEDNOISE_H
#define AUD_CLOCKEDNOISE_H
#include <stdint.h>
#include <stdlib.h>
#include "aud_dsp.h"

typedef struct {
    float phase;
    float sample;
    float next_sample;
    float frequency;
    float sample_rate;
} Aud_ClockedNoise;

void  Aud_ClockedNoise_Init(Aud_ClockedNoise *self, float sample_rate);
float Aud_ClockedNoise_Process(Aud_ClockedNoise *self);
void  Aud_ClockedNoise_SetFreq(Aud_ClockedNoise *self, float freq);
void  Aud_ClockedNoise_Sync(Aud_ClockedNoise *self);

#endif /* AUD_CLOCKEDNOISE_H */
