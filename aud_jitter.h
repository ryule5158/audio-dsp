/* SPDX-License-Identifier: LGPL-2.1-only */
/**
 * @file    aud_jitter.h
 * @brief   Randomly segmented line generator (jitter modulator).
 *          Ported from DaisySP Utility/jitter (LGPL). Original by Paul Batchelor.
 */
#ifndef AUD_JITTER_H
#define AUD_JITTER_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float   amp, cps_min, cps_max, cps, sample_rate;
    int32_t phs;
    bool    init_flag;
    float   num1, num2, dfd_max;
} Aud_Jitter;

void  Aud_Jitter_Init(Aud_Jitter *self, float sample_rate);
float Aud_Jitter_Process(Aud_Jitter *self);
void  Aud_Jitter_SetCpsMin(Aud_Jitter *self, float cps_min);
void  Aud_Jitter_SetCpsMax(Aud_Jitter *self, float cps_max);
void  Aud_Jitter_SetAmp(Aud_Jitter *self, float amp);

#endif /* AUD_JITTER_H */
