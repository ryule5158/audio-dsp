#include "aud_tremolo.h"
#include <stddef.h>
#include "aud_dsp.h"

void Aud_Tremolo_Init(Aud_Tremolo *self, float sample_rate)
{
    if (self == NULL) return;
    self->sample_rate = sample_rate;
    Aud_Osc_Init(&self->osc, sample_rate);
    self->dc_os = 0.5f;
    Aud_Osc_SetAmp(&self->osc, 0.5f);
    Aud_Osc_SetFreq(&self->osc, 1.0f);
}

float Aud_Tremolo_Process(Aud_Tremolo *self, float in)
{
    if (self == NULL) return in;
    float modsig = self->dc_os + Aud_Osc_Process(&self->osc);
    return in * modsig;
}

void Aud_Tremolo_SetFreq(Aud_Tremolo *self, float freq)
{
    if (self == NULL) return;
    Aud_Osc_SetFreq(&self->osc, freq);
}

void Aud_Tremolo_SetWaveform(Aud_Tremolo *self, int waveform)
{
    if (self == NULL) return;
    Aud_Osc_SetWaveform(&self->osc, (uint8_t)waveform);
}

void Aud_Tremolo_SetDepth(Aud_Tremolo *self, float depth)
{
    if (self == NULL) return;
    depth = aud_fclamp(depth, 0.0f, 1.0f) * 0.5f;
    Aud_Osc_SetAmp(&self->osc, depth);
    self->dc_os = 1.0f - depth;
}
