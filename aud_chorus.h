#ifndef AUD_CHORUS_H
#define AUD_CHORUS_H
#include <stdint.h>
#include "aud_delayline.h"
#define AUD_CHORUS_DELAY_LEN 2400
typedef struct {float sr,lfo_phase,lfo_freq,lfo_amp,feedback,delay;float del_buf[AUD_CHORUS_DELAY_LEN];Aud_DelayLine del;} Aud_ChorusEngine;
typedef struct {Aud_ChorusEngine engines[2];float gain_frac,pan[2],sigl,sigr;} Aud_Chorus;

void Aud_ChorusEngine_Init(Aud_ChorusEngine *self,float sr);
float Aud_ChorusEngine_Process(Aud_ChorusEngine *self,float in);
void Aud_ChorusEngine_SetLfoDepth(Aud_ChorusEngine *self,float d);
void Aud_ChorusEngine_SetLfoFreq(Aud_ChorusEngine *self,float f);
void Aud_ChorusEngine_SetDelay(Aud_ChorusEngine *self,float d);
void Aud_ChorusEngine_SetDelayMs(Aud_ChorusEngine *self,float ms);
void Aud_ChorusEngine_SetFeedback(Aud_ChorusEngine *self,float fb);

void Aud_Chorus_Init(Aud_Chorus *self,float sr);
float Aud_Chorus_Process(Aud_Chorus *self,float in);
float Aud_Chorus_GetLeft(Aud_Chorus *self);
float Aud_Chorus_GetRight(Aud_Chorus *self);
void Aud_Chorus_SetPan(Aud_Chorus *self,float l,float r);
void Aud_Chorus_SetLfoDepth(Aud_Chorus *self,float l,float r);
void Aud_Chorus_SetLfoFreq(Aud_Chorus *self,float l,float r);
void Aud_Chorus_SetDelay(Aud_Chorus *self,float l,float r);
void Aud_Chorus_SetDelayMs(Aud_Chorus *self,float l,float r);
void Aud_Chorus_SetFeedback(Aud_Chorus *self,float l,float r);
#endif
