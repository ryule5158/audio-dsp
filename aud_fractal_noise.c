#include "aud_fractal_noise.h"
#include "aud_dsp.h"
#include <stddef.h>

void Aud_FractalRandomGenerator_Init(Aud_FractalRandomGenerator *self, float sample_rate)
{
    if (self == NULL) return;
    self->sample_rate = sample_rate;
    self->decay       = 0.5f;
    self->frequency   = 440.0f / sample_rate;
    for (int i = 0; i < AUD_FRACTAL_ORDER; i++) {
        Aud_ClockedNoise_Init(&self->generator[i], sample_rate);
    }
}

float Aud_FractalRandomGenerator_Process(Aud_FractalRandomGenerator *self)
{
    if (self == NULL) return 0.0f;
    float gain      = 0.5f;
    float sum       = 0.0f;
    float frequency = self->frequency;

    for (int i = 0; i < AUD_FRACTAL_ORDER; i++) {
        Aud_ClockedNoise_SetFreq(&self->generator[i], frequency * self->sample_rate);
        sum += Aud_ClockedNoise_Process(&self->generator[i]) * gain;
        gain     *= self->decay;
        frequency *= 2.0f;
    }
    return sum;
}

void Aud_FractalRandomGenerator_SetFreq(Aud_FractalRandomGenerator *self, float freq)
{
    if (self == NULL) return;
    self->frequency = aud_fclamp(freq / self->sample_rate, 0.0f, 1.0f);
}

void Aud_FractalRandomGenerator_SetColor(Aud_FractalRandomGenerator *self, float color)
{
    if (self == NULL) return;
    self->decay = aud_fclamp(color, 0.0f, 1.0f);
}
