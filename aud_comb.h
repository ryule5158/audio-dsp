/* SPDX-License-Identifier: LGPL-2.1-only */
/**
 * @file    aud_comb.h
 * @brief   Comb filter (feedforward / feedback).
 *          Ported from DaisySP Filters/comb (MIT)
 */
#ifndef AUD_COMB_H
#define AUD_COMB_H
#include <stdint.h>

typedef struct {
    float buffer[4096];
    float gain;
    uint32_t delay;
    uint32_t max_size;
    uint32_t write_ptr;
    uint8_t  rev;     /* 1 = feedback, 0 = feedforward */
} Aud_Comb;

void Aud_Comb_Init(Aud_Comb *c, uint32_t max_delay);
void Aud_Comb_SetDelay(Aud_Comb *c, uint32_t delay);
void Aud_Comb_SetGain(Aud_Comb *c, float gain);
void Aud_Comb_SetRev(Aud_Comb *c, uint8_t reverse);
float Aud_Comb_Process(Aud_Comb *c, float in);

#endif /* AUD_COMB_H */
