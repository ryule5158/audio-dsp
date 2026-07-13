#include "aud_clockednoise.h"
#include <stddef.h>

void Aud_ClockedNoise_Init(Aud_ClockedNoise *self, float sample_rate)
{
    if (self == NULL) return;
    self->sample_rate = sample_rate;
    self->phase       = 0.0f;
    self->sample      = 0.0f;
    self->next_sample = 0.0f;
    self->frequency   = 0.001f;
}

float Aud_ClockedNoise_Process(Aud_ClockedNoise *self)
{
    if (self == NULL) return 0.0f;
    float next_sample = self->next_sample;
    float sample      = self->sample;

    float this_sample = next_sample;
    next_sample       = 0.0f;

    const float raw_sample = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
    float       raw_amount = 4.0f * (self->frequency - 0.25f);
    raw_amount             = aud_fclamp(raw_amount, 0.0f, 1.0f);

    self->phase += self->frequency;

    if (self->phase >= 1.0f) {
        self->phase -= 1.0f;
        float t             = self->phase / self->frequency;
        float new_sample    = raw_sample;
        float discontinuity = new_sample - sample;
        this_sample += aud_this_blep(t) * discontinuity;
        next_sample += aud_next_blep(t) * discontinuity;
        sample = new_sample;
    }

    next_sample += sample;
    self->next_sample = next_sample;
    self->sample      = sample;

    return this_sample + raw_amount * (raw_sample - this_sample);
}

void Aud_ClockedNoise_SetFreq(Aud_ClockedNoise *self, float freq)
{
    if (self == NULL) return;
    float f = freq / self->sample_rate;
    self->frequency = aud_fclamp(f, 0.0f, 1.0f);
}

void Aud_ClockedNoise_Sync(Aud_ClockedNoise *self)
{
    if (self == NULL) return;
    self->phase = 1.0f;
}
