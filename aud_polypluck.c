/* SPDX-License-Identifier: LGPL-2.1-only */
#include "aud_polypluck.h"
#include <stddef.h>
#include <string.h>
void Aud_PolyPluck_Init(Aud_PolyPluck *self,float sr,int n_voices,int mode){if(self==NULL)return;
 self->n_voices=(n_voices>AUD_POLYPLUCK_MAX_VOICES)?AUD_POLYPLUCK_MAX_VOICES:n_voices;if(self->n_voices<1)self->n_voices=1;
 for(int i=0;i<self->n_voices;i++){Aud_Pluck_Init(&self->pluck[i],sr,self->buf[i],1024,mode);
  Aud_DcBlock_Init(&self->dc[i],sr);self->active[i]=false;}}
float Aud_PolyPluck_Process(Aud_PolyPluck *self,float trig){if(self==NULL)return 0.0f;float sum=0.0f;
 for(int i=0;i<self->n_voices;i++){
  float t=(i==0&&trig!=0.0f)?trig:0.0f;float s=Aud_Pluck_Process(&self->pluck[i],&t);sum+=Aud_DcBlock_Process(&self->dc[i],s);}
 return sum/(float)self->n_voices;}
void Aud_PolyPluck_SetFreq(Aud_PolyPluck *self,int voice,float freq){if(self&&voice>=0&&voice<self->n_voices)Aud_Pluck_SetFreq(&self->pluck[voice],freq);}
void Aud_PolyPluck_SetDecay(Aud_PolyPluck *self,int voice,float decay){if(self&&voice>=0&&voice<self->n_voices)Aud_Pluck_SetDecay(&self->pluck[voice],decay);}
void Aud_PolyPluck_SetDamp(Aud_PolyPluck *self,int voice,float damp){if(self&&voice>=0&&voice<self->n_voices)Aud_Pluck_SetDamp(&self->pluck[voice],damp);}
