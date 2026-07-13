/**
 * @file    aud_smooth_random.h
 * @brief   Smooth random generator (slewed noise for modulation).
 *          Ported from DaisySP Utility/smooth_random (MIT)
 *          Original by Emilie Gillet, 2016
 */
#ifndef AUD_SMOOTH_RANDOM_H
#define AUD_SMOOTH_RANDOM_H
#include <stdint.h>
#include <stdlib.h>
#include "aud_dsp.h"

typedef struct {
    float frequency;  /* Normalized: freq_hz / sample_rate */
    float phase;
    float from;
    float interval;
    float sample_rate;
} Aud_SmoothRandomGenerator;

static inline void Aud_SmoothRandomGenerator_Init(
    Aud_SmoothRandomGenerator *self, float sample_rate)
{
    if (self == NULL) return;
    self->sample_rate = sample_rate;
    self->phase       = 0.0f;
    self->from        = 0.0f;
    self->interval    = 0.0f;
    self->frequency   = 1.0f / sample_rate;
}

static inline void Aud_SmoothRandomGenerator_SetFreq(
    Aud_SmoothRandomGenerator *self, float freq)
{
    if (self == NULL) return;
    float f = freq / self->sample_rate;
    self->frequency = aud_fclamp(f, 0.0f, 1.0f);
}

static inline float Aud_SmoothRandomGenerator_Process(
    Aud_SmoothRandomGenerator *self)
{
    if (self == NULL) return 0.0f;
    self->phase += self->frequency;
    if (self->phase >= 1.0f) {
        self->phase -= 1.0f;
        self->from += self->interval;
        self->interval = ((float)rand() / (float)RAND_MAX) * 2.0f
                       - 1.0f - self->from;
    }
    float t = self->phase * self->phase * (3.0f - 2.0f * self->phase);
    return self->from + self->interval * t;
}

#endif /* AUD_SMOOTH_RANDOM_H */
