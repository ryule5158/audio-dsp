#include "aud_variablesawosc.h"
#include <stddef.h>
#include "aud_dsp.h"
#include <math.h>
#define kVariableSawNotchDepth 0.2f

static float aud_varsaw_ComputeNaive(float phase, float pw, float slope_up,
    float slope_down, float tri_amt, float notch_amt)
{
    float notch_saw = (phase < pw) ? phase : (1.0f + kVariableSawNotchDepth);
    float triangle  = (phase < pw) ? (phase * slope_up)
                                   : (1.0f - (phase - pw) * slope_down);
    return notch_saw * notch_amt + triangle * tri_amt;
}

void Aud_VariableSawOscillator_Init(Aud_VariableSawOscillator *self, float sr)
{
    if (self == NULL) return;
    self->sample_rate = sr;
    self->phase = self->next_sample = 0.0f;
    self->previous_pw = 0.5f; self->high = false;
    self->frequency = 220.0f / sr;
    self->pw = 0.5f; self->waveshape = 1.0f;
}

float Aud_VariableSawOscillator_Process(Aud_VariableSawOscillator *self)
{
    if (self == NULL) return 0.0f;
    float next_sample = self->next_sample;
    float this_sample = next_sample; next_sample = 0.0f;

    const float tri_amt  = self->waveshape;
    const float notch_amt = 1.0f - self->waveshape;
    const float slope_up  = 1.0f / self->pw;
    const float slope_down= 1.0f / (1.0f - self->pw);

    self->phase += self->frequency;
    if (!self->high && self->phase >= self->pw) {
        const float tri_step = (slope_up + slope_down) * self->frequency * tri_amt;
        const float notch = (kVariableSawNotchDepth + 1.0f - self->pw) * notch_amt;
        const float t = (self->phase - self->pw) / (self->previous_pw - self->pw + self->frequency);
        this_sample += notch * aud_this_blep(t);
        next_sample += notch * aud_next_blep(t);
        this_sample -= tri_step * aud_this_integrated_blep(t);
        next_sample -= tri_step * aud_next_integrated_blep(t);
        self->high = true;
    } else if (self->phase >= 1.0f) {
        self->phase -= 1.0f;
        const float tri_step = (slope_up + slope_down) * self->frequency * tri_amt;
        const float notch = (kVariableSawNotchDepth + 1.0f) * notch_amt;
        const float t = self->phase / self->frequency;
        this_sample -= notch * aud_this_blep(t);
        next_sample -= notch * aud_next_blep(t);
        this_sample += tri_step * aud_this_integrated_blep(t);
        next_sample += tri_step * aud_next_integrated_blep(t);
        self->high = false;
    }
    next_sample += aud_varsaw_ComputeNaive(self->phase, self->pw,
        slope_up, slope_down, tri_amt, notch_amt);
    self->previous_pw = self->pw;
    self->next_sample = next_sample;
    return (2.0f * this_sample - 1.0f) / (1.0f + kVariableSawNotchDepth);
}

void Aud_VariableSawOscillator_SetFreq(Aud_VariableSawOscillator *self, float f)
{
    if (self == NULL) return;
    f = f / self->sample_rate;
    f = (f >= 0.25f) ? 0.25f : f;
    if (f >= 0.25f) self->pw = 0.5f;
    self->frequency = f;
}

void Aud_VariableSawOscillator_SetPW(Aud_VariableSawOscillator *self, float pw)
{
    if (self == NULL) return;
    if (self->frequency >= 0.25f) { self->pw = 0.5f; }
    else { self->pw = aud_fclamp(pw, self->frequency * 2.0f,
                                  1.0f - 2.0f * self->frequency); }
}

void Aud_VariableSawOscillator_SetWaveshape(Aud_VariableSawOscillator *self, float ws)
    { if (self) self->waveshape = ws; }
