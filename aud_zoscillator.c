#include "aud_zoscillator.h"
#include <stddef.h>
#include "aud_dsp.h"
#include <math.h>
#include <stdbool.h>

static float zosc_Sine(float p) { return sinf(AUD_TWOPI * p); }

static float zosc_Z(float c, float d, float f, float shape, float mode)
{
    float ramp_down = 0.5f * (1.0f + zosc_Sine(0.5f * d + 0.25f));
    float offset, phase_shift;
    if (mode < 0.333f) {
        offset = 1.0f; phase_shift = 0.25f + mode * 1.50f;
    } else if (mode < 0.666f) {
        phase_shift = 0.7495f - (mode - 0.33f) * 0.75f;
        offset = -zosc_Sine(phase_shift);
    } else {
        phase_shift = 0.7495f - (mode - 0.33f) * 0.75f;
        offset = 0.001f;
    }
    float discontinuity = zosc_Sine(f + phase_shift);
    float contour;
    if (shape < 0.5f) {
        shape *= 2.0f;
        if (c >= 0.5f) ramp_down *= shape;
        contour = 1.0f + (zosc_Sine(c + 0.25f) - 1.0f) * shape;
    } else {
        contour = zosc_Sine(c + shape * 0.5f);
    }
    return (ramp_down * (offset + discontinuity) - offset) * contour;
}

void Aud_ZOscillator_Init(Aud_ZOscillator *self, float sr)
{
    if (self == NULL) return;
    self->sample_rate = sr;
    self->carrier_phase = self->discontinuity_phase = self->formant_phase = 0.0f;
    self->next_sample = 0.0f;
    self->carrier_frequency = 220.0f / sr;
    self->formant_frequency = 550.0f / sr;
    self->carrier_shape = 1.0f; self->shape_new = 1.0f;
    self->mode = 0.0f; self->mode_new = 0.0f;
}

float Aud_ZOscillator_Process(Aud_ZOscillator *self)
{
    if (self == NULL) return 0.0f;
    float ns = self->next_sample, ts = ns; ns = 0.0f;
    bool reset = false; float rt = 0.0f;

    self->discontinuity_phase += 2.0f * self->carrier_frequency;
    self->carrier_phase += self->carrier_frequency;
    reset = self->discontinuity_phase >= 1.0f;

    if (reset) {
        self->discontinuity_phase -= 1.0f;
        rt = self->discontinuity_phase / (2.0f * self->carrier_frequency);
        float cpb = (self->carrier_phase >= 1.0f) ? 1.0f : 0.5f;
        float cpa = (self->carrier_phase >= 1.0f) ? 0.0f : 0.5f;
        float ms = self->mode + (1.0f - rt) * (self->mode - self->mode_new);
        float ss = self->carrier_shape + (1.0f - rt) * (self->carrier_shape - self->shape_new);
        float before = zosc_Z(cpb, 1.0f,
            self->formant_phase + (1.0f - rt) * self->formant_frequency, ss, ms);
        float after = zosc_Z(cpa, 0.0f, 0.0f, self->shape_new, self->mode_new);
        float disc = after - before;
        ts += disc * aud_this_blep(rt); ns += disc * aud_next_blep(rt);
        self->formant_phase = rt * self->formant_frequency;
        if (self->carrier_phase > 1.0f) self->carrier_phase = self->discontinuity_phase * 0.5f;
    } else {
        self->formant_phase += self->formant_frequency;
        if (self->formant_phase >= 1.0f) self->formant_phase -= 1.0f;
    }
    if (self->carrier_phase >= 1.0f) self->carrier_phase -= 1.0f;
    self->carrier_shape = self->shape_new; self->mode = self->mode_new;
    ns += zosc_Z(self->carrier_phase, self->discontinuity_phase,
        self->formant_phase, self->carrier_shape, self->mode);
    self->next_sample = ns;
    return ts;
}

void Aud_ZOscillator_SetFreq(Aud_ZOscillator *self, float f)
    { if (self) { self->carrier_frequency = f/self->sample_rate;
      if (self->carrier_frequency >= 0.25f) self->carrier_frequency = 0.25f; } }
void Aud_ZOscillator_SetFormantFreq(Aud_ZOscillator *self, float f)
    { if (self) { self->formant_frequency = f/self->sample_rate;
      if (self->formant_frequency >= 0.25f) self->formant_frequency = 0.25f; } }
void Aud_ZOscillator_SetShape(Aud_ZOscillator *self, float s)
    { if (self) self->shape_new = s; }
void Aud_ZOscillator_SetMode(Aud_ZOscillator *self, float m)
    { if (self) self->mode_new = m; }
