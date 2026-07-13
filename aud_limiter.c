/**
 * @file    aud_limiter.c
 * @brief   Ported from DaisySP Dynamics/limiter.cpp (MIT)
 *          Original: STMLib peak limiter with SLOPE envelope follower
 */
#include "aud_limiter.h"
#include <math.h>
#include <stddef.h>

#define SLOPE(out, in_val, positive, negative)          \
    do {                                                 \
        float error_ = (in_val) - (out);                 \
        (out) += (error_ > 0.0f ? (positive) : (negative)) * error_; \
    } while(0)

void Aud_Limiter_Init(Aud_Limiter *self)
{
    if (self == NULL) return;
    self->peak = 0.5f;
}

void Aud_Limiter_ProcessBlock(Aud_Limiter *self, float *in, uint32_t size, float pre_gain)
{
    if (self == NULL || in == NULL) return;
    while (size--) {
        float pre  = *in * pre_gain;
        float peak = fabsf(pre);
        SLOPE(self->peak, peak, 0.05f, 0.00002f);
        float gain = (self->peak <= 1.0f ? 1.0f : 1.0f / self->peak);
        *in++      = aud_soft_limit(pre * gain * 0.7f);
    }
}
