/**
 * @file    aud_fm2.h
 * @brief   2-operator FM synthesis.
 *          Ported from DaisySP Synthesis/fm2 (MIT)
 */
#ifndef AUD_FM2_H
#define AUD_FM2_H
#include <stdint.h>
#define AUD_FM2_KIDX_SCALAR 0.2f
#define AUD_FM2_KIDX_SCALAR_RECIP 5.0f

typedef struct {
    float sample_rate;
    float ratio, lratio;   /* Modulator / carrier frequency ratio */
    float freq, lfreq;     /* Carrier frequency */
    float idx;             /* FM index (modulation depth) */
    float phase_car;       /* Carrier phase */
    float phase_mod;       /* Modulator phase */
    float inc_car;         /* Carrier increment per sample */
    float inc_mod;         /* Modulator increment per sample */
} Aud_Fm2;

void  Aud_Fm2_Init(Aud_Fm2 *fm, float sample_rate);
float Aud_Fm2_Process(Aud_Fm2 *fm);
void  Aud_Fm2_SetFrequency(Aud_Fm2 *fm, float freq);
void  Aud_Fm2_SetRatio(Aud_Fm2 *fm, float ratio);
void  Aud_Fm2_SetIndex(Aud_Fm2 *fm, float index);
float Aud_Fm2_GetIndex(Aud_Fm2 *fm);
void  Aud_Fm2_Reset(Aud_Fm2 *fm);

#endif /* AUD_FM2_H */
