/**
 * @file    aud_crossfade.c
 * @brief   Ported from DaisySP Dynamics/crossfade.cpp (MIT)
 */
#include "aud_crossfade.h"
#include "aud_dsp.h"
#include <math.h>
#include <stddef.h>

#define REALLYSMALLFLOAT 0.000001f

void Aud_CrossFade_Init(Aud_CrossFade *self, int curve)
{
    if (self == NULL) return;
    self->pos   = 0.5f;
    self->curve = (uint8_t)(curve < AUD_CROSSFADE_LAST ? curve : AUD_CROSSFADE_LIN);
}

void Aud_CrossFade_SetPos(Aud_CrossFade *self, float pos)
{
    if (self == NULL) return;
    self->pos = aud_fclamp(pos, 0.0f, 1.0f);
}

void Aud_CrossFade_SetCurve(Aud_CrossFade *self, uint8_t curve)
{
    if (self == NULL) return;
    self->curve = curve;
}

float Aud_CrossFade_GetPos(Aud_CrossFade *self)
{
    return (self != NULL) ? self->pos : 0.5f;
}

uint8_t Aud_CrossFade_GetCurve(Aud_CrossFade *self)
{
    return (self != NULL) ? self->curve : AUD_CROSSFADE_LIN;
}

float Aud_CrossFade_Process(Aud_CrossFade *self, float in1, float in2)
{
    if (self == NULL) return in1;
    float scalar_1, scalar_2;
    switch (self->curve) {
        case AUD_CROSSFADE_LIN:
            scalar_1 = self->pos;
            return (in1 * (1.0f - scalar_1)) + (in2 * scalar_1);

        case AUD_CROSSFADE_CPOW:
            scalar_1 = sinf(self->pos * AUD_HALFPI);
            scalar_2 = sinf((1.0f - self->pos) * AUD_HALFPI);
            return (in1 * scalar_2) + (in2 * scalar_1);

        case AUD_CROSSFADE_LOG: {
            const float kCrossLogMin = logf(REALLYSMALLFLOAT);
            const float kCrossLogMax = logf(1.0f);
            scalar_1 = expf(self->pos * (kCrossLogMax - kCrossLogMin) + kCrossLogMin);
            return (in1 * (1.0f - scalar_1)) + (in2 * scalar_1);
        }

        case AUD_CROSSFADE_EXP:
            scalar_1 = self->pos * self->pos;
            return (in1 * (1.0f - scalar_1)) + (in2 * scalar_1);

        default:
            return 0.0f;
    }
}
