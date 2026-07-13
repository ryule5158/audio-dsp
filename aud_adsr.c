#include "aud_adsr.h"
#include <stddef.h>
#include <math.h>

#ifndef M_E
#define M_E 2.71828182845904523536f
#endif

static void adsr_SetTimeConstant(float timeInS, float *time, float *coeff, float sample_rate)
{
    if (timeInS != *time) {
        *time = timeInS;
        if (*time > 0.0f) {
            const float target = logf(1.0f / M_E);
            *coeff = 1.0f - expf(target / (*time * sample_rate));
        } else {
            *coeff = 1.0f; /* instant change */
        }
    }
}

void Aud_Adsr_Init(Aud_Adsr *self, float sample_rate, int blockSize)
{
    if (self == NULL) return;
    self->sample_rate  = sample_rate / blockSize;
    self->attackShape  = -1.0f;
    self->attackTarget = 0.0f;
    self->attackTime   = -1.0f;
    self->decayTime    = -1.0f;
    self->releaseTime  = -1.0f;
    self->sus_level    = 0.7f;
    self->x            = 0.0f;
    self->gate         = false;
    self->mode         = AUD_ADSR_SEG_IDLE;

    Aud_Adsr_SetTime(self, AUD_ADSR_SEG_ATTACK, 0.1f);
    Aud_Adsr_SetTime(self, AUD_ADSR_SEG_DECAY, 0.1f);
    Aud_Adsr_SetTime(self, AUD_ADSR_SEG_RELEASE, 0.1f);
}

void Aud_Adsr_Retrigger(Aud_Adsr *self, bool hard)
{
    if (self == NULL) return;
    self->mode = AUD_ADSR_SEG_ATTACK;
    if (hard) self->x = 0.0f;
}

void Aud_Adsr_SetTime(Aud_Adsr *self, int seg, float time)
{
    if (self == NULL) return;
    switch (seg) {
        case AUD_ADSR_SEG_ATTACK: Aud_Adsr_SetAttackTime(self, time, 0.0f); break;
        case AUD_ADSR_SEG_DECAY:  adsr_SetTimeConstant(time, &self->decayTime, &self->decayD0, self->sample_rate); break;
        case AUD_ADSR_SEG_RELEASE:adsr_SetTimeConstant(time, &self->releaseTime, &self->releaseD0, self->sample_rate); break;
        default: return;
    }
}

void Aud_Adsr_SetAttackTime(Aud_Adsr *self, float timeInS, float shape)
{
    if (self == NULL) return;
    if ((timeInS != self->attackTime) || (shape != self->attackShape)) {
        self->attackTime  = timeInS;
        self->attackShape = shape;
        if (timeInS > 0.0f) {
            float x         = shape;
            float target    = 9.0f * powf(x, 10.0f) + 0.3f * x + 1.01f;
            self->attackTarget = target;
            float logTarget = logf(1.0f - (1.0f / target));
            self->attackD0  = 1.0f - expf(logTarget / (timeInS * self->sample_rate));
        } else {
            self->attackD0 = 1.0f;
        }
    }
}

void Aud_Adsr_SetDecayTime(Aud_Adsr *self, float timeInS)
    { if (self) adsr_SetTimeConstant(timeInS, &self->decayTime, &self->decayD0, self->sample_rate); }
void Aud_Adsr_SetReleaseTime(Aud_Adsr *self, float timeInS)
    { if (self) adsr_SetTimeConstant(timeInS, &self->releaseTime, &self->releaseD0, self->sample_rate); }
void Aud_Adsr_SetSustainLevel(Aud_Adsr *self, float sus_level)
{
    if (self == NULL) return;
    sus_level = (sus_level <= 0.0f) ? -0.01f : ((sus_level > 1.0f) ? 1.0f : sus_level);
    self->sus_level = sus_level;
}
uint8_t Aud_Adsr_GetCurrentSegment(const Aud_Adsr *self) { return (self != NULL) ? self->mode : AUD_ADSR_SEG_IDLE; }
bool Aud_Adsr_IsRunning(const Aud_Adsr *self) { return (self != NULL) && (self->mode != AUD_ADSR_SEG_IDLE); }

float Aud_Adsr_Process(Aud_Adsr *self, bool gate)
{
    if (self == NULL) return 0.0f;
    float out = 0.0f;

    if (gate && !self->gate)
        self->mode = AUD_ADSR_SEG_ATTACK;
    else if (!gate && self->gate)
        self->mode = AUD_ADSR_SEG_RELEASE;
    self->gate = gate;

    float D0 = self->attackD0;
    if (self->mode == AUD_ADSR_SEG_DECAY)
        D0 = self->decayD0;
    else if (self->mode == AUD_ADSR_SEG_RELEASE)
        D0 = self->releaseD0;

    float target = (self->mode == AUD_ADSR_SEG_DECAY) ? self->sus_level : -0.01f;

    switch (self->mode) {
        case AUD_ADSR_SEG_IDLE: out = 0.0f; break;
        case AUD_ADSR_SEG_ATTACK:
            self->x += D0 * (self->attackTarget - self->x);
            out = self->x;
            if (out > 1.0f) { self->x = out = 1.0f; self->mode = AUD_ADSR_SEG_DECAY; }
            break;
        case AUD_ADSR_SEG_DECAY:
        case AUD_ADSR_SEG_RELEASE:
            self->x += D0 * (target - self->x);
            out = self->x;
            if (out < 0.0f) { self->x = out = 0.0f; self->mode = AUD_ADSR_SEG_IDLE; }
            break;
        default: break;
    }
    return out;
}
