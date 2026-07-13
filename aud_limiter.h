/**
 * @file    aud_limiter.h
 * @brief   Simple peak limiter — block processing with SLOPE envelope.
 *          Ported from DaisySP Dynamics/limiter (MIT)
 *          Original from pichenettes/stmlib (Emilie Gillet)
 */
#ifndef AUD_LIMITER_H
#define AUD_LIMITER_H
#include <stdint.h>
#include "aud_dsp.h"

typedef struct {
    float peak;
} Aud_Limiter;

void Aud_Limiter_Init(Aud_Limiter *self);
void Aud_Limiter_ProcessBlock(Aud_Limiter *self, float *in, uint32_t size, float pre_gain);

#endif
