/**
 * @file    aud_crossfade.h
 * @brief   Crossfade between two signals — LIN / CPOW / LOG / EXP curves.
 *          Ported from DaisySP Dynamics/crossfade (MIT)
 *          Original by Paul Batchelor, ported from Soundpipe
 */
#ifndef AUD_CROSSFADE_H
#define AUD_CROSSFADE_H
#include <stdint.h>

enum {
    AUD_CROSSFADE_LIN,
    AUD_CROSSFADE_CPOW,
    AUD_CROSSFADE_LOG,
    AUD_CROSSFADE_EXP,
    AUD_CROSSFADE_LAST
};

typedef struct {
    float   pos;
    uint8_t curve;
} Aud_CrossFade;

void  Aud_CrossFade_Init(Aud_CrossFade *self, int curve);
void  Aud_CrossFade_SetPos(Aud_CrossFade *self, float pos);
void  Aud_CrossFade_SetCurve(Aud_CrossFade *self, uint8_t curve);
float Aud_CrossFade_GetPos(Aud_CrossFade *self);
uint8_t Aud_CrossFade_GetCurve(Aud_CrossFade *self);
float Aud_CrossFade_Process(Aud_CrossFade *self, float in1, float in2);

#endif
