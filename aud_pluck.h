/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef AUD_PLUCK_H
#define AUD_PLUCK_H
#include <stdint.h>
enum { AUD_PLUCK_MODE_RECURSIVE, AUD_PLUCK_MODE_WEIGHTED_AVERAGE, AUD_PLUCK_LAST };
typedef struct {float amp,freq,decay,damp,ifreq,sicps,sample_rate;int32_t phs256,npts,maxpts,mode;float*buf;char init;} Aud_Pluck;
void Aud_Pluck_Init(Aud_Pluck *self,float sr,float*buf,int32_t npts,int32_t mode);
float Aud_Pluck_Process(Aud_Pluck *self,float*trig);
void Aud_Pluck_SetAmp(Aud_Pluck *self,float a);
void Aud_Pluck_SetFreq(Aud_Pluck *self,float f);
void Aud_Pluck_SetDecay(Aud_Pluck *self,float d);
void Aud_Pluck_SetDamp(Aud_Pluck *self,float d);
void Aud_Pluck_SetMode(Aud_Pluck *self,int32_t m);
#endif
