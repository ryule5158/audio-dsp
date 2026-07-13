#include "aud_formantosc.h"
#include <stddef.h>
#include "aud_dsp.h"
#include <math.h>

static float aud_formant_Sine(float phase) { return sinf(phase * AUD_TWOPI); }

void Aud_FormantOscillator_Init(Aud_FormantOscillator *self, float sample_rate)
{
    if (self == NULL) return;
    self->carrier_phase = self->formant_phase = self->next_sample = 0.0f;
    self->carrier_frequency = 0.0f;
    self->formant_frequency = 100.0f / sample_rate;
    self->phase_shift = 0.0f;
    self->sample_rate = sample_rate;
}

float Aud_FormantOscillator_Process(Aud_FormantOscillator *self)
{
    if (self == NULL) return 0.0f;
    float this_sample = self->next_sample;
    float next_sample = 0.0f;
    self->carrier_phase += self->carrier_frequency;

    if (self->carrier_phase >= 1.0f) {
        self->carrier_phase -= 1.0f;
        float reset_time = self->carrier_phase / self->carrier_frequency;
        float formant_phase_at_reset = self->formant_phase
            + (1.0f - reset_time) * self->formant_frequency;
        float before = aud_formant_Sine(formant_phase_at_reset
            + self->phase_shift + (self->ps_inc * (1.0f - reset_time)));
        float after = aud_formant_Sine(self->phase_shift + self->ps_inc);
        float discontinuity = after - before;
        this_sample += discontinuity * aud_this_blep(reset_time);
        next_sample += discontinuity * aud_next_blep(reset_time);
        self->formant_phase = reset_time * self->formant_frequency;
    } else {
        self->formant_phase += self->formant_frequency;
        if (self->formant_phase >= 1.0f) self->formant_phase -= 1.0f;
    }

    self->phase_shift += self->ps_inc;
    self->ps_inc = 0.0f;
    next_sample += aud_formant_Sine(self->formant_phase + self->phase_shift);
    self->next_sample = next_sample;
    return this_sample;
}

void Aud_FormantOscillator_SetFormantFreq(Aud_FormantOscillator *self, float f)
{
    if (self == NULL) return;
    self->formant_frequency = f / self->sample_rate;
    if (self->formant_frequency >= 0.25f) self->formant_frequency = 0.25f;
    if (self->formant_frequency <= -0.25f) self->formant_frequency = -0.25f;
}

void Aud_FormantOscillator_SetCarrierFreq(Aud_FormantOscillator *self, float f)
{
    if (self == NULL) return;
    self->carrier_frequency = f / self->sample_rate;
    if (self->carrier_frequency >= 0.25f) self->carrier_frequency = 0.25f;
    if (self->carrier_frequency <= -0.25f) self->carrier_frequency = -0.25f;
}

void Aud_FormantOscillator_SetPhaseShift(Aud_FormantOscillator *self, float ps)
{
    if (self == NULL) return;
    self->ps_inc = ps - self->phase_shift;
}
