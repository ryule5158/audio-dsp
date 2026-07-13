#include "aud_flanger.h"
#include <stddef.h>
#include "aud_dsp.h"

static float flanger_ProcessLfo(Aud_Flanger *self)
{
    if (self == NULL) return 0.0f;
    self->lfo_phase += self->lfo_freq;
    if (self->lfo_phase > 1.0f) {
        self->lfo_phase = 1.0f - (self->lfo_phase - 1.0f);
        self->lfo_freq *= -1.0f;
    } else if (self->lfo_phase < -1.0f) {
        self->lfo_phase = -1.0f - (self->lfo_phase + 1.0f);
        self->lfo_freq *= -1.0f;
    }
    return self->lfo_phase * self->lfo_amp;
}

void Aud_Flanger_Init(Aud_Flanger *self, float sr)
{
    if (self == NULL) return;
    self->sample_rate = sr;
    self->feedback = 0.194f; /* 0.2 * 0.97 */
    Aud_DelayLine_Init(&self->del, self->del_buf, AUD_FLANGER_DELAY_LEN);
    self->lfo_amp = 0.0f;
    self->delay = 5.275f; /* .75 maps to ~5.275ms → ~253 samples at 48k */
    self->lfo_phase = 0.0f;
    self->lfo_freq = 0.025f; /* .3 / sr * 4, roughly */
    self->lfo_amp = self->delay * 0.9f;
}

float Aud_Flanger_Process(Aud_Flanger *self, float in)
{
    if (self == NULL) return in;
    float lfo = flanger_ProcessLfo(self);
    Aud_DelayLine_SetDelay(&self->del, 1.0f + lfo + self->delay);
    float out = Aud_DelayLine_Read(&self->del);
    Aud_DelayLine_Write(&self->del, in + out * self->feedback);
    return (in + out) * 0.5f;
}

void Aud_Flanger_SetFeedback(Aud_Flanger *self, float fb)
    { if (self) { self->feedback = aud_fclamp(fb, 0.0f, 1.0f) * 0.97f; } }
void Aud_Flanger_SetLfoDepth(Aud_Flanger *self, float d)
    { if (self) { d = aud_fclamp(d, 0.0f, 0.93f); self->lfo_amp = d * self->delay; } }
void Aud_Flanger_SetLfoFreq(Aud_Flanger *self, float f)
    { if (self) { f = 4.0f * f / self->sample_rate;
      f *= (self->lfo_freq < 0.0f) ? -1.0f : 1.0f;
      self->lfo_freq = aud_fclamp(f, -0.25f, 0.25f); } }
void Aud_Flanger_SetDelay(Aud_Flanger *self, float d)
    { if (self) Aud_Flanger_SetDelayMs(self, 0.1f + d * 6.9f); }
void Aud_Flanger_SetDelayMs(Aud_Flanger *self, float ms)
    { if (self) { ms = aud_fmax(0.1f, ms);
      self->delay = ms * 0.001f * self->sample_rate;
      self->lfo_amp = aud_fmin(self->lfo_amp, self->delay); } }
