#include "aud_adenv.h"
#include <math.h>
#include <float.h>
#include <stddef.h>

/* Fast exp approximation (10x multiply — from original DaisySP) */
static float adenv_expf_fast(float x)
{
    x = 1.0f + x / 1024.0f;
    x *= x; x *= x; x *= x; x *= x; x *= x;
    x *= x; x *= x; x *= x; x *= x; x *= x;
    return x;
}

void Aud_AdEnv_Init(Aud_AdEnv *self, float sample_rate)
{
    if (self == NULL) return;
    self->sample_rate     = sample_rate;
    self->current_segment = AUD_ADENV_SEG_IDLE;
    self->curve_scalar    = 0.0f;
    self->phase           = 0;
    self->min             = 0.0f;
    self->max             = 1.0f;
    self->output          = 0.0001f;
    for (uint8_t i = 0; i < AUD_ADENV_SEG_LAST; i++) {
        self->segment_time[i] = 0.05f;
    }
}

float Aud_AdEnv_Process(Aud_AdEnv *self)
{
    if (self == NULL) return 0.0f;
    uint32_t time_samps;
    float val, out, beg, end, inc;

    /* Handle retriggering */
    if (self->trigger) {
        self->trigger         = 0;
        self->current_segment = AUD_ADENV_SEG_ATTACK;
        self->phase           = 0;
        self->curve_x         = 0.0f;
        self->retrig_val      = self->output;
    }

    time_samps = (uint32_t)(self->segment_time[self->current_segment] * self->sample_rate);

    switch (self->current_segment) {
        case AUD_ADENV_SEG_ATTACK: beg = self->retrig_val; end = 1.0f; break;
        case AUD_ADENV_SEG_DECAY:  beg = 1.0f;            end = 0.0f; break;
        default:                   beg = 0.0f;            end = 0.0f; break;
    }

    if (self->prev_segment != self->current_segment) {
        self->curve_x = 0.0f;
        self->phase   = 0;
    }

    if (self->curve_scalar == 0.0f) {
        self->c_inc = (end - beg) / (float)time_samps;
    } else {
        self->c_inc = (end - beg) / (1.0f - adenv_expf_fast(self->curve_scalar));
    }

    if (self->c_inc >= 0.0f) {
        if (self->c_inc < FLT_EPSILON) self->c_inc = FLT_EPSILON;
    } else {
        if (self->c_inc > -FLT_EPSILON) self->c_inc = -FLT_EPSILON;
    }

    val = self->output;
    inc = self->c_inc;
    out = val;

    if (self->curve_scalar == 0.0f) {
        val += inc;
    } else {
        self->curve_x += (self->curve_scalar / (float)time_samps);
        val = beg + inc * (1.0f - adenv_expf_fast(self->curve_x));
        if (val != val) val = 0.0f; /* NaN check */
    }

    self->phase += 1;
    self->prev_segment = self->current_segment;

    if (self->current_segment != AUD_ADENV_SEG_IDLE) {
        if ((out >= 1.0f && self->current_segment == AUD_ADENV_SEG_ATTACK)
         || (out <= 0.0f && self->current_segment == AUD_ADENV_SEG_DECAY)) {
            self->current_segment++;
            if (self->current_segment > AUD_ADENV_SEG_DECAY) {
                self->current_segment = AUD_ADENV_SEG_IDLE;
            }
        }
    }
    if (self->current_segment == AUD_ADENV_SEG_IDLE) {
        val = out = 0.0f;
    }
    self->output = val;
    return out * (self->max - self->min) + self->min;
}

void Aud_AdEnv_Trigger(Aud_AdEnv *self)
    { if (self) self->trigger = 1; }
void Aud_AdEnv_SetTime(Aud_AdEnv *self, uint8_t seg, float time)
    { if (self && seg < AUD_ADENV_SEG_LAST) self->segment_time[seg] = time; }
void Aud_AdEnv_SetCurve(Aud_AdEnv *self, float scalar)
    { if (self) self->curve_scalar = scalar; }
void Aud_AdEnv_SetMin(Aud_AdEnv *self, float min)
    { if (self) self->min = min; }
void Aud_AdEnv_SetMax(Aud_AdEnv *self, float max)
    { if (self) self->max = max; }
float Aud_AdEnv_GetValue(const Aud_AdEnv *self)
    { return (self != NULL) ? (self->output * (self->max - self->min) + self->min) : 0.0f; }
uint8_t Aud_AdEnv_GetCurrentSegment(const Aud_AdEnv *self)
    { return (self != NULL) ? self->current_segment : AUD_ADENV_SEG_IDLE; }
bool Aud_AdEnv_IsRunning(const Aud_AdEnv *self)
    { return (self != NULL) && (self->current_segment != AUD_ADENV_SEG_IDLE); }
