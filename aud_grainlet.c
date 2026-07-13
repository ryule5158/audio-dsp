#include "aud_grainlet.h"
#include <stddef.h>
#include "aud_dsp.h"
#include <math.h>

/* ---- internal helpers (was private methods in C++) ---- */
static float aud_grainlet_Sine(float phase)
{
    return sinf(phase * AUD_TWOPI);
}

static float aud_grainlet_Carrier(float phase, float shape)
{
    shape *= 3.0f;
    int   shape_integral   = (int)shape;
    float shape_fractional = shape - (float)shape_integral;
    float t = 1.0f - shape_fractional;

    if (shape_integral == 0) {
        phase = phase * (1.0f + t * t * t * 15.0f);
        if (phase >= 1.0f) phase = 1.0f;
        phase += 0.75f;
    } else if (shape_integral == 1) {
        float breakpoint = 0.001f + 0.499f * t * t * t;
        if (phase < breakpoint) {
            phase *= (0.5f / breakpoint);
        } else {
            phase = 0.5f + (phase - breakpoint) * 0.5f / (1.0f - breakpoint);
        }
        phase += 0.75f;
    } else {
        t     = 1.0f - t;
        phase = 0.25f + phase * (0.5f + t * t * t * 14.5f);
        if (phase >= 0.75f) phase = 0.75f;
    }
    return (aud_grainlet_Sine(phase) + 1.0f) * 0.25f;
}

static float aud_grainlet_Grainlet(float carrier_phase, float formant_phase,
                                    float shape, float bleed)
{
    float carrier = aud_grainlet_Carrier(carrier_phase, shape);
    float formant = aud_grainlet_Sine(formant_phase);
    return carrier * (formant + bleed) / (1.0f + bleed);
}

/* ---- public API ---- */
void Aud_GrainletOscillator_Init(Aud_GrainletOscillator *self, float sample_rate)
{
    if (self == NULL) return;
    self->sample_rate = sample_rate;
    self->carrier_phase = 0.0f;
    self->formant_phase = 0.0f;
    self->next_sample   = 0.0f;
    self->carrier_shape = 0.0f;
    self->carrier_bleed = 0.0f;

    Aud_GrainletOscillator_SetFreq(self, 440.0f);
    Aud_GrainletOscillator_SetFormantFreq(self, 220.0f);
    Aud_GrainletOscillator_SetShape(self, 0.5f);
    Aud_GrainletOscillator_SetBleed(self, 0.5f);
}

float Aud_GrainletOscillator_Process(Aud_GrainletOscillator *self)
{
    if (self == NULL) return 0.0f;
    float this_sample = self->next_sample;
    float next_sample = 0.0f;

    self->carrier_phase += self->carrier_frequency;

    if (self->carrier_phase >= 1.0f) {
        self->carrier_phase -= 1.0f;
        float reset_time = self->carrier_phase / self->carrier_frequency;

        float shape_inc = self->new_carrier_shape - self->carrier_shape;
        float bleed_inc = self->new_carrier_bleed - self->carrier_bleed;

        float before = aud_grainlet_Grainlet(
            1.0f,
            self->formant_phase + (1.0f - reset_time) * self->formant_frequency,
            self->new_carrier_shape + shape_inc * (1.0f - reset_time),
            self->new_carrier_bleed + bleed_inc * (1.0f - reset_time));

        float after = aud_grainlet_Grainlet(
            0.0f, 0.0f,
            self->new_carrier_shape, self->new_carrier_bleed);

        float discontinuity = after - before;
        this_sample += aud_this_blep(reset_time) * discontinuity;
        next_sample += aud_next_blep(reset_time) * discontinuity;
        self->formant_phase = reset_time * self->formant_frequency;
    } else {
        self->formant_phase += self->formant_frequency;
        if (self->formant_phase >= 1.0f) {
            self->formant_phase -= 1.0f;
        }
    }

    self->carrier_bleed = self->new_carrier_bleed;
    self->carrier_shape = self->new_carrier_shape;
    next_sample += aud_grainlet_Grainlet(
        self->carrier_phase, self->formant_phase,
        self->carrier_shape, self->carrier_bleed);
    self->next_sample = next_sample;
    return this_sample;
}

void Aud_GrainletOscillator_SetFreq(Aud_GrainletOscillator *self, float freq)
{
    if (self == NULL) return;
    self->carrier_frequency = freq / self->sample_rate;
    if (self->carrier_frequency > 0.5f) self->carrier_frequency = 0.5f;
}

void Aud_GrainletOscillator_SetFormantFreq(Aud_GrainletOscillator *self, float freq)
{
    if (self == NULL) return;
    self->formant_frequency = freq / self->sample_rate;
    if (self->formant_frequency > 0.5f) self->formant_frequency = 0.5f;
}

void Aud_GrainletOscillator_SetShape(Aud_GrainletOscillator *self, float shape)
{
    if (self == NULL) return;
    self->new_carrier_shape = shape;
}

void Aud_GrainletOscillator_SetBleed(Aud_GrainletOscillator *self, float bleed)
{
    if (self == NULL) return;
    self->new_carrier_bleed = bleed;
}
