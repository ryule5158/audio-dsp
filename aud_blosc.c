/* SPDX-License-Identifier: LGPL-2.1-only */
#include "aud_blosc.h"
#include <math.h>
#include <stddef.h>
static float blosc_ProcessSquare(Aud_BlOsc *self){float fS2=fmin(2047.0f,self->sampling_freq*(self->pw/self->freq));
 float fS5=(float)((int)fS2+1)-fS2,fS6=self->quarter_sr/self->freq,fS7=self->sec_per_sample*self->freq,fS8=fS2-(int)fS2;
 self->rec0[0]=fmodf(self->rec0[1]+fS7,1.0f);float t0=2.0f*self->rec0[0]-1.0f;t0*=t0;self->vec1[0]=t0;
 float t1=fS6*(t0-self->vec1[1]);self->vec2[self->iota&4095]=t1;
 float out=self->amp*(0.0f-((fS5*self->vec2[(self->iota-(int)fS2)&4095]+fS8*self->vec2[(self->iota-((int)fS2+1))&4095])-t1));
 self->rec0[1]=self->rec0[0];self->vec1[1]=self->vec1[0];self->iota++;return out;}
static float blosc_ProcessTriangle(Aud_BlOsc *self){float fS1=self->four_over_sr*(self->amp*self->freq),fS3=self->half_sr/self->freq;
 int iS4=(int)fS3,iS5=1+iS4;float fS6=iS5-fS3,fS7=self->quarter_sr/self->freq,fS8=self->sec_per_sample*self->freq,fS9=fS3-iS4;
 self->rec1[0]=fmodf(fS8+self->rec1[1],1.0f);float t0=2.0f*self->rec1[0]-1.0f;t0*=t0;self->vec1[0]=t0;
 float t1=fS7*(t0-self->vec1[1]);self->vec2[self->iota&4095]=t1;
 self->rec0[0]=0.0f-((fS6*self->vec2[(self->iota-iS4)&4095]+fS9*self->vec2[(self->iota-iS5)&4095])-(0.999f*self->rec0[1]+t1));
 float out=fS1*self->rec0[0];self->rec1[1]=self->rec1[0];self->rec0[1]=self->rec0[0];self->vec1[1]=self->vec1[0];self->iota++;return out;}
static float blosc_ProcessSaw(Aud_BlOsc *self){float fS1=self->sampling_freq*(self->amp/self->freq),fS2=self->two_over_sr*self->freq,fS3=self->sampling_freq/self->freq;
 self->rec0[0]=fmodf(1.0f+self->rec0[1],fS3);float t0=fS2*self->rec0[0]-1.0f;t0*=t0;self->vec0[0]=t0;self->vec1[0]=0.25f;
 float out=fS1*((t0-self->vec0[1])*self->vec1[1]);self->rec0[1]=self->rec0[0];self->vec0[1]=self->vec0[0];self->vec1[1]=self->vec1[0];return out;}
void Aud_BlOsc_Init(Aud_BlOsc *self,float sr){if(self==NULL)return;int i;self->sampling_freq=sr;self->half_sr=0.5f*sr;
 self->quarter_sr=sr*0.25f;self->sec_per_sample=1.0f/sr;self->two_over_sr=2.0f/sr;self->four_over_sr=4.0f/sr;
 self->freq=440.0f;self->amp=0.5f;self->pw=0.5f;self->iota=0;self->mode=AUD_BLOSC_WAVE_TRIANGLE;
 for(i=0;i<2;i++)self->rec0[i]=self->rec1[i]=self->vec0[i]=self->vec1[i]=0.0f;
 for(i=0;i<4096;i++)self->vec2[i]=0.0f;}
void Aud_BlOsc_Reset(Aud_BlOsc *self){if(self==NULL)return;int i;self->iota=0;
 for(i=0;i<2;i++)self->rec0[i]=self->rec1[i]=self->vec0[i]=self->vec1[i]=0.0f;
 for(i=0;i<4096;i++)self->vec2[i]=0.0f;}
float Aud_BlOsc_Process(Aud_BlOsc *self){if(self==NULL)return 0.0f;switch(self->mode){
 case AUD_BLOSC_WAVE_TRIANGLE:return blosc_ProcessTriangle(self);case AUD_BLOSC_WAVE_SAW:return blosc_ProcessSaw(self);
 case AUD_BLOSC_WAVE_SQUARE:return blosc_ProcessSquare(self);default:return 0.0f;}}
void Aud_BlOsc_SetFreq(Aud_BlOsc *self,float f){if(self)self->freq=f;}
void Aud_BlOsc_SetAmp(Aud_BlOsc *self,float a){if(self)self->amp=a;}
void Aud_BlOsc_SetPw(Aud_BlOsc *self,float pw){if(self)self->pw=1.0f-pw;}
void Aud_BlOsc_SetWaveform(Aud_BlOsc *self,uint8_t wf){if(self)self->mode=wf;}
