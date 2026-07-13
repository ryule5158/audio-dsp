#include "aud_harmonic_osc.h"
#include <math.h>
#include <stddef.h>

static bool harmonic_cmp(float a, float b)
{
    return fabsf(a - b) > 0.000001f;
}

void Aud_HarmonicOscillator_Init(Aud_HarmonicOscillator *self, float sample_rate)
{
    if (self == NULL) return;

    self->sample_rate_ = sample_rate;
    self->phase_       = 0.0f;
    for (int i = 0; i < AUD_HARMONIC_OSC_NUM_HARMONICS; ++i) {
        self->amplitude_[i]    = 0.0f;
        self->newamplitude_[i] = 0.0f;
    }

    self->amplitude_[0]    = 1.0f;
    self->newamplitude_[0] = 1.0f;
    self->frequency_       = 0.0f;
    self->first_harmonic_index_ = 1;
    self->recalc_          = false;

    Aud_HarmonicOscillator_SetFirstHarmIdx(self, 1);
    Aud_HarmonicOscillator_SetFreq(self, 440.0f);
    self->recalc_ = false;
}

float Aud_HarmonicOscillator_Process(Aud_HarmonicOscillator *self)
{
    if (self == NULL) return 0.0f;

    if (self->recalc_) {
        self->recalc_ = false;
        for (int i = 0; i < AUD_HARMONIC_OSC_NUM_HARMONICS; ++i) {
            float f = self->frequency_ * (float)(self->first_harmonic_index_ + i);
            if (f >= 0.5f) f = 0.5f;
            self->amplitude_[i] = self->newamplitude_[i] * (1.0f - f * 2.0f);
        }
    }

    self->phase_ += self->frequency_;
    if (self->phase_ >= 1.0f) self->phase_ -= 1.0f;

    const float two_x = 2.0f * sinf(self->phase_ * AUD_TWOPI);
    float previous;
    float current;

    if (self->first_harmonic_index_ == 1) {
        previous = 1.0f;
        current  = two_x * 0.5f;
    } else {
        const float k = (float)self->first_harmonic_index_;
        previous = sinf((self->phase_ * (k - 1.0f) + 0.25f) * AUD_TWOPI);
        current  = sinf((self->phase_ * k) * AUD_TWOPI);
    }

    float sum = 0.0f;
    for (int i = 0; i < AUD_HARMONIC_OSC_NUM_HARMONICS; ++i) {
        sum += self->amplitude_[i] * current;
        float temp = current;
        current    = two_x * current - previous;
        previous   = temp;
    }

    return sum;
}

void Aud_HarmonicOscillator_SetFreq(Aud_HarmonicOscillator *self, float freq)
{
    if (self == NULL || self->sample_rate_ <= 0.0f) return;

    freq = freq / self->sample_rate_;
    if (freq >= 0.5f) freq = 0.5f;
    if (freq <= -0.5f) freq = -0.5f;
    self->recalc_ = harmonic_cmp(freq, self->frequency_) || self->recalc_;
    self->frequency_ = freq;
}

void Aud_HarmonicOscillator_SetFirstHarmIdx(Aud_HarmonicOscillator *self, int idx)
{
    if (self == NULL) return;
    if (idx < 1) idx = 1;
    self->recalc_ = (idx != self->first_harmonic_index_) || self->recalc_;
    self->first_harmonic_index_ = idx;
}

void Aud_HarmonicOscillator_SetAmplitudes(Aud_HarmonicOscillator *self, const float *amplitudes)
{
    if (self == NULL || amplitudes == NULL) return;
    for (int i = 0; i < AUD_HARMONIC_OSC_NUM_HARMONICS; ++i) {
        self->recalc_ = harmonic_cmp(self->newamplitude_[i], amplitudes[i]) || self->recalc_;
        self->newamplitude_[i] = amplitudes[i];
    }
}

void Aud_HarmonicOscillator_SetSingleAmp(Aud_HarmonicOscillator *self, float amp, int idx)
{
    if (self == NULL || idx < 0 || idx >= AUD_HARMONIC_OSC_NUM_HARMONICS) return;
    self->recalc_ = harmonic_cmp(self->newamplitude_[idx], amp) || self->recalc_;
    self->newamplitude_[idx] = amp;
}

void Aud_HarmonicOscillatorillator_Init(Aud_HarmonicOscillatorillator *self, float sample_rate)
{
    Aud_HarmonicOscillator_Init(self, sample_rate);
}

float Aud_HarmonicOscillatorillator_Process(Aud_HarmonicOscillatorillator *self, float in)
{
    (void)in;
    return Aud_HarmonicOscillator_Process(self);
}

void Aud_HarmonicOscillatorillator_SetFreq(Aud_HarmonicOscillatorillator *self, float freq)
{
    Aud_HarmonicOscillator_SetFreq(self, freq);
}
