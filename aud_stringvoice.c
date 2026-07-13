#include "aud_stringvoice.h"
#include "aud_dsp.h"
#include <stdlib.h>
#include <stddef.h>

void Aud_StringVoice_Init(Aud_StringVoice *self, float sr)
{
    if (self == NULL) return;
    self->sample_rate = sr;
    Aud_Svf_Init(&self->excitation_filter, sr);
    Aud_String_Init(&self->string, sr);
    Aud_Dust_Init(&self->dust);
    self->remaining_noise_samples = 0;
    self->sustain = false; self->trig = false;
    self->f0 = 440.0f / sr; self->accent = 0.8f;
    self->brightness = 0.2f; self->damping = 0.7f;
    self->density = self->brightness * self->brightness;
}

void Aud_StringVoice_Reset(Aud_StringVoice *self) { if (self) Aud_String_Reset(&self->string); }
void Aud_StringVoice_SetSustain(Aud_StringVoice *self, bool s) { if (self) self->sustain = s; }
void Aud_StringVoice_Trig(Aud_StringVoice *self) { if (self) self->trig = true; }
void Aud_StringVoice_SetFreq(Aud_StringVoice *self, float f) { if (self) {
    Aud_String_SetFreq(&self->string, f); self->f0 = f / self->sample_rate;
    self->f0 = aud_fclamp(self->f0, 0.0f, 0.25f); } }
void Aud_StringVoice_SetAccent(Aud_StringVoice *self, float a) { if (self) self->accent = aud_fclamp(a, 0.0f, 1.0f); }
void Aud_StringVoice_SetStructure(Aud_StringVoice *self, float s) { if (self) {
    s = aud_fclamp(s, 0.0f, 1.0f); float nl = (s < 0.24f) ? (s - 0.24f) * 4.166f
        : ((s > 0.26f) ? (s - 0.26f) * 1.35135f : 0.0f);
    Aud_String_SetNonLinearity(&self->string, nl); } }
void Aud_StringVoice_SetBrightness(Aud_StringVoice *self, float b) { if (self) {
    self->brightness = aud_fclamp(b, 0.0f, 1.0f);
    self->density = self->brightness * self->brightness; } }
void Aud_StringVoice_SetDamping(Aud_StringVoice *self, float d) { if (self) self->damping = aud_fclamp(d, 0.0f, 1.0f); }
float Aud_StringVoice_GetAux(Aud_StringVoice *self) { return (self != NULL) ? self->aux : 0.0f; }

float Aud_StringVoice_Process(Aud_StringVoice *self, bool trigger)
{
    if (self == NULL) return 0.0f;
    const float brightness = self->brightness + 0.25f * self->accent * (1.0f - self->brightness);
    const float damping    = self->damping + 0.25f * self->accent * (1.0f - self->damping);

    if (trigger || self->trig || self->sustain) {
        self->trig = false;
        const float range = 72.0f, f = 4.0f * self->f0;
        const float cutoff = aud_fmin(f * powf(2.0f,
            (1.0f / 12.0f) * (brightness * (2.0f - brightness) - 0.5f) * range), 0.499f);
        const float q = self->sustain ? 1.0f : 0.5f;
        self->remaining_noise_samples = (uint32_t)(1.0f / self->f0);
        Aud_Svf_SetFreq(&self->excitation_filter, cutoff * self->sample_rate);
        Aud_Svf_SetRes(&self->excitation_filter, q);
    }

    float temp = 0.0f;
    if (self->sustain) {
        const float dust_f = 0.00005f + 0.99995f * self->density * self->density;
        Aud_Dust_SetDensity(&self->dust, dust_f);
        temp = Aud_Dust_Process(&self->dust) * (8.0f - dust_f * 6.0f) * self->accent;
    } else if (self->remaining_noise_samples) {
        temp = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
        self->remaining_noise_samples--;
    }

    Aud_Svf_Process(&self->excitation_filter, temp);
    temp = Aud_Svf_Low(&self->excitation_filter);
    self->aux = temp;

    Aud_String_SetBrightness(&self->string, brightness);
    Aud_String_SetDamping(&self->string, damping);
    return Aud_String_Process(&self->string, temp);
}
