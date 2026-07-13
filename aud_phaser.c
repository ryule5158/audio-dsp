#include "aud_phaser.h"
#include <stddef.h>
#include "aud_dsp.h"

static float phaser_ProcessLfo(Aud_PhaserEngine *self)
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
    return self->lfo_phase * self->lfo_amp * self->ap_freq;
}

/* ---- PhaserEngine ---- */
void Aud_PhaserEngine_Init(Aud_PhaserEngine *self, float sr)
{
    if (self == NULL) return;
    self->sample_rate = sr;
    Aud_DelayLine_Init(&self->del, self->del_buf, AUD_PHASER_DELAY_LEN);
    self->lfo_amp = 0.0f; self->feedback = 0.2f; self->ap_freq = 200.0f;
    Aud_DelayLine_SetDelay(&self->del, 0.0f);
    self->os = 30.0f; self->deltime = 0.0f;
    self->last_sample = 0.0f; self->lfo_phase = 0.0f;
    self->lfo_freq = 0.025f; self->lfo_amp = 0.9f;
}

float Aud_PhaserEngine_Process(Aud_PhaserEngine *self, float in)
{
    if (self == NULL) return in;
    float lfo = phaser_ProcessLfo(self);
    aud_fonepole(&self->deltime, self->sample_rate / (lfo + self->ap_freq + self->os), 0.0001f);

    /* Allpass via delay line: y = -coeff * write + read, write = in + coeff * read */
    float rd = Aud_DelayLine_ReadAt(&self->del, self->deltime);
    float wr = in + self->feedback * self->last_sample;
    float ap = -0.3f * wr + rd;
    Aud_DelayLine_Write(&self->del, wr);
    self->last_sample = ap;

    return (in + self->last_sample) * 0.5f;
}

void Aud_PhaserEngine_SetLfoDepth(Aud_PhaserEngine *self, float d)
    { if (self) self->lfo_amp = aud_fclamp(d, 0.0f, 1.0f); }
void Aud_PhaserEngine_SetLfoFreq(Aud_PhaserEngine *self, float f)
    { if (self) { f = 4.0f * f / self->sample_rate;
      f *= (self->lfo_freq < 0.0f) ? -1.0f : 1.0f;
      self->lfo_freq = aud_fclamp(f, -0.25f, 0.25f); } }
void Aud_PhaserEngine_SetFreq(Aud_PhaserEngine *self, float f)
    { if (self) self->ap_freq = aud_fclamp(f, 0.0f, 20000.0f); }
void Aud_PhaserEngine_SetFeedback(Aud_PhaserEngine *self, float fb)
    { if (self) self->feedback = aud_fclamp(fb, 0.0f, 0.75f); }

/* ---- Phaser ---- */
void Aud_Phaser_Init(Aud_Phaser *self, float sr)
{
    if (self == NULL) return;
    for (int i = 0; i < AUD_PHASER_MAX_POLES; i++)
        Aud_PhaserEngine_Init(&self->engines[i], sr);
    self->poles = 4; self->gain_frac = 0.5f;
}

float Aud_Phaser_Process(Aud_Phaser *self, float in)
{
    if (self == NULL) return in;
    float sig = 0.0f;
    for (int i = 0; i < self->poles; i++)
        sig += Aud_PhaserEngine_Process(&self->engines[i], in);
    return sig;
}

void Aud_Phaser_SetPoles(Aud_Phaser *self, int p)
    { if (self) self->poles = (p < 1) ? 1 : ((p > 8) ? 8 : p); }
void Aud_Phaser_SetLfoDepth(Aud_Phaser *self, float d)
    { if (self) for (int i=0; i<AUD_PHASER_MAX_POLES; i++) Aud_PhaserEngine_SetLfoDepth(&self->engines[i], d); }
void Aud_Phaser_SetLfoFreq(Aud_Phaser *self, float f)
    { if (self) for (int i=0; i<AUD_PHASER_MAX_POLES; i++) Aud_PhaserEngine_SetLfoFreq(&self->engines[i], f); }
void Aud_Phaser_SetFreq(Aud_Phaser *self, float f)
    { if (self) for (int i=0; i<AUD_PHASER_MAX_POLES; i++) Aud_PhaserEngine_SetFreq(&self->engines[i], f); }
void Aud_Phaser_SetFeedback(Aud_Phaser *self, float fb)
    { if (self) for (int i=0; i<AUD_PHASER_MAX_POLES; i++) Aud_PhaserEngine_SetFeedback(&self->engines[i], fb); }
