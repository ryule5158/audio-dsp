#include "aud_particle.h"
#include <stddef.h>
#include "aud_dsp.h"
#include <math.h>

void Aud_Particle_Init(Aud_Particle *self, float sample_rate)
{
    if (self == NULL) return;
    self->sample_rate = sample_rate;
    self->sync        = false;
    self->aux         = 0.0f;
    self->frequency   = 440.0f / sample_rate;
    self->resonance   = 0.9f;
    self->density     = 0.5f;
    self->gain        = 1.0f;
    self->spread      = 1.0f;
    self->rand_freq   = sample_rate / 48.0f / sample_rate;
    self->rand_phase  = 0.0f;
    self->pre_gain    = 0.0f;
    Aud_Svf_Init(&self->filter, sample_rate);
    Aud_Svf_SetDrive(&self->filter, 0.7f);
}

float Aud_Particle_Process(Aud_Particle *self)
{
    if (self == NULL) return 0.0f;
    float u = ((float)rand() / (float)RAND_MAX);
    float s = 0.0f;

    if (u <= self->density || self->sync) {
        s = (u <= self->density) ? u * self->gain : s;
        self->rand_phase += self->rand_freq;

        if (self->rand_phase >= 1.0f || self->sync) {
            self->rand_phase = (self->rand_phase >= 1.0f)
                             ? (self->rand_phase - 1.0f)
                             : self->rand_phase;

            const float u2 = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
            const float f  = aud_fmin(
                powf(2.0f, (1.0f / 12.0f) * self->spread * u2)
                    * self->frequency,
                0.25f);
            self->pre_gain = 0.5f / sqrtf(
                self->resonance * f * sqrtf(self->density));
            Aud_Svf_SetFreq(&self->filter, f * self->sample_rate);
            Aud_Svf_SetRes(&self->filter, self->resonance);
        }
    }
    self->aux = s;

    Aud_Svf_Process(&self->filter, self->pre_gain * s);
    return Aud_Svf_Band(&self->filter);
}

float Aud_Particle_GetNoise(Aud_Particle *self)
{
    return (self != NULL) ? self->aux : 0.0f;
}

void Aud_Particle_SetFreq(Aud_Particle *self, float freq)
{
    if (self == NULL) return;
    freq /= self->sample_rate;
    self->frequency = aud_fclamp(freq, 0.0f, 1.0f);
}

void Aud_Particle_SetResonance(Aud_Particle *self, float resonance)
{
    if (self == NULL) return;
    self->resonance = aud_fclamp(resonance, 0.0f, 1.0f);
}

void Aud_Particle_SetRandomFreq(Aud_Particle *self, float freq)
{
    if (self == NULL) return;
    freq /= self->sample_rate;
    self->rand_freq = aud_fclamp(freq, 0.0f, 1.0f);
}

void Aud_Particle_SetDensity(Aud_Particle *self, float density)
{
    if (self == NULL) return;
    self->density = aud_fclamp(density * 0.3f, 0.0f, 1.0f);
}

void Aud_Particle_SetGain(Aud_Particle *self, float gain)
{
    if (self == NULL) return;
    self->gain = aud_fclamp(gain, 0.0f, 1.0f);
}

void Aud_Particle_SetSpread(Aud_Particle *self, float spread)
{
    if (self == NULL) return;
    self->spread = (spread < 0.0f) ? 0.0f : spread;
}

void Aud_Particle_SetSync(Aud_Particle *self, bool sync)
{
    if (self == NULL) return;
    self->sync = sync;
}
