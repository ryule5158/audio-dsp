#include "aud_vosim.h"
#include <stddef.h>
#include "aud_dsp.h"
#include <math.h>

static float aud_vosim_Sine(float p) { return sinf(AUD_TWOPI * p); }

void Aud_VosimOscillator_Init(Aud_VosimOscillator *self, float sr)
{
    if (self == NULL) return;
    self->sample_rate = sr;
    self->carrier_phase = self->formant_1_phase = self->formant_2_phase = 0.0f;
    self->carrier_frequency = 105.0f / sr;
    self->formant_1_frequency = 1390.0f / sr;
    self->formant_2_frequency = 817.0f / sr;
    self->carrier_shape = 0.5f;
}

float Aud_VosimOscillator_Process(Aud_VosimOscillator *self)
{
    if (self == NULL) return 0.0f;
    self->carrier_phase += self->carrier_frequency;
    if (self->carrier_phase >= 1.0f) {
        self->carrier_phase -= 1.0f;
        float rt = self->carrier_phase / self->carrier_frequency;
        self->formant_1_phase = rt * self->formant_1_frequency;
        self->formant_2_phase = rt * self->formant_2_frequency;
    } else {
        self->formant_1_phase += self->formant_1_frequency;
        if (self->formant_1_phase >= 1.0f) self->formant_1_phase -= 1.0f;
        self->formant_2_phase += self->formant_2_frequency;
        if (self->formant_2_phase >= 1.0f) self->formant_2_phase -= 1.0f;
    }
    float carrier = aud_vosim_Sine(self->carrier_phase * 0.5f + 0.25f) + 1.0f;
    float reset_phase = 0.75f - 0.25f * self->carrier_shape;
    float reset_amp = aud_vosim_Sine(reset_phase);
    float f0 = aud_vosim_Sine(self->formant_1_phase + reset_phase) - reset_amp;
    float f1 = aud_vosim_Sine(self->formant_2_phase + reset_phase) - reset_amp;
    return carrier * (f0 + f1) * 0.25f + reset_amp;
}

void Aud_VosimOscillator_SetFreq(Aud_VosimOscillator *self, float f)
    { if (self) { self->carrier_frequency = f/self->sample_rate;
      if (self->carrier_frequency > 0.25f) self->carrier_frequency = 0.25f; } }
void Aud_VosimOscillator_SetForm1Freq(Aud_VosimOscillator *self, float f)
    { if (self) { self->formant_1_frequency = f/self->sample_rate;
      if (self->formant_1_frequency > 0.25f) self->formant_1_frequency = 0.25f; } }
void Aud_VosimOscillator_SetForm2Freq(Aud_VosimOscillator *self, float f)
    { if (self) { self->formant_2_frequency = f/self->sample_rate;
      if (self->formant_2_frequency > 0.25f) self->formant_2_frequency = 0.25f; } }
void Aud_VosimOscillator_SetShape(Aud_VosimOscillator *self, float s)
    { if (self) self->carrier_shape = s; }
