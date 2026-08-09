/* SPDX-License-Identifier: LGPL-2.1-only */
/**
 * @file    aud_allpass.h
 * @brief   Allpass filter — passes all frequencies, shifts phase.
 *          Ported from DaisySP Filters/allpass (MIT)
 */
#ifndef AUD_ALLPASS_H
#define AUD_ALLPASS_H
#include <stdint.h>

typedef struct {
    float buffer[2048];  /* Internal ring buffer */
    float gain;          /* Allpass coefficient */
    uint32_t delay;      /* Delay in samples */
    uint32_t max_size;
    uint32_t write_ptr;
} Aud_Allpass;

void Aud_Allpass_Init(Aud_Allpass *ap, uint32_t max_delay);
void Aud_Allpass_SetDelay(Aud_Allpass *ap, uint32_t delay);
void Aud_Allpass_SetGain(Aud_Allpass *ap, float gain);
float Aud_Allpass_Process(Aud_Allpass *ap, float in);

#endif /* AUD_ALLPASS_H */
