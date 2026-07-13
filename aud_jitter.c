#include "aud_jitter.h"
#include <stddef.h>
#include <math.h>
#include <stdlib.h>

#define FT_MAXLEN 0x1000000L
#define FT_PHMASK 0x0FFFFFFL

/* ---- internal helpers ---- */
static float aud_jitter_randGab(void)
{
    return (float)((rand() >> 1) & 0x7fffffff)
           * 4.656612875245796924105750827168e-10f;
}

static float aud_jitter_biRandGab(void)
{
    return (float)(rand() & 0x7fffffff)
           * 4.656612875245796924105750827168e-10f;
}

static void aud_jitter_Reset(Aud_Jitter *self)
{
    if (self == NULL) return;
    self->num2      = aud_jitter_biRandGab();
    self->init_flag = true;
    self->phs       = 0;
}

/* ---- public API ---- */
void Aud_Jitter_Init(Aud_Jitter *self, float sample_rate)
{
    if (self == NULL) return;
    self->sample_rate = sample_rate;
    self->amp         = 0.5f;
    self->cps_min     = 0.5f;
    self->cps_max     = 4.0f;
    aud_jitter_Reset(self);
}

float Aud_Jitter_Process(Aud_Jitter *self)
{
    if (self == NULL) return 0.0f;

    if (self->init_flag) {
        self->init_flag = false;
        float out  = self->num2 * self->amp;
        self->cps  = aud_jitter_randGab() * (self->cps_max - self->cps_min)
                   + self->cps_min;
        self->phs &= FT_PHMASK;
        self->num1    = self->num2;
        self->num2    = aud_jitter_biRandGab();
        self->dfd_max = 1.0f * (self->num2 - self->num1) / (float)FT_MAXLEN;
        return out;
    }

    float out = (self->num1 + (float)self->phs * self->dfd_max) * self->amp;
    self->phs += (int32_t)(self->cps * ((float)FT_MAXLEN / self->sample_rate));

    if (self->phs >= FT_MAXLEN) {
        self->cps  = aud_jitter_randGab() * (self->cps_max - self->cps_min)
                   + self->cps_min;
        self->phs &= FT_PHMASK;
        self->num1    = self->num2;
        self->num2    = aud_jitter_biRandGab();
        self->dfd_max = 1.0f * (self->num2 - self->num1) / (float)FT_MAXLEN;
    }

    return out;
}

void Aud_Jitter_SetAmp(Aud_Jitter *self, float amp)
{
    if (self == NULL) return;
    self->amp = amp;
    aud_jitter_Reset(self);
}

void Aud_Jitter_SetCpsMin(Aud_Jitter *self, float cps_min)
{
    if (self == NULL) return;
    self->cps_min = cps_min;
    aud_jitter_Reset(self);
}

void Aud_Jitter_SetCpsMax(Aud_Jitter *self, float cps_max)
{
    if (self == NULL) return;
    self->cps_max = cps_max;
    aud_jitter_Reset(self);
}
