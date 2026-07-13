#include "aud_granularplayer.h"
#include <math.h>
#include <stddef.h>
static uint32_t gp_WrapIdx(uint32_t idx,uint32_t sz){return (idx>sz)?idx-sz:idx;}
static float gp_CentsToRatio(float c){return powf(2.0f,c/1200.0f);}
static float gp_NegativeInvert(Aud_Phasor *phs,float freq){float p=Aud_Phasor_Process(phs);return(freq>0.0f)?p:(-p+1.0f);}
void Aud_GranularPlayer_Init(Aud_GranularPlayer *self,float*sample,int size,float sr){if(self==NULL)return;self->sample=sample;
 self->size=size;self->sample_rate=sr;Aud_Phasor_Init(&self->phs,sr,0.0f,0.0f);Aud_Phasor_Init(&self->phsImp,sr,0.0f,0.0f);
 Aud_Phasor_Init(&self->phs2,sr,0.0f,0.5f);Aud_Phasor_Init(&self->phsImp2,sr,0.0f,0.0f);
 self->sample_frequency=sr/(float)size;for(int i=0;i<256;i++)self->cosEnv[i]=sinf((i/256.0f)*3.14159265358979f);}
float Aud_GranularPlayer_Process(Aud_GranularPlayer *self,float speed,float transposition,float grain_size){if(self==NULL)return 0.0f;
 self->grain_size=grain_size;self->speed=speed*self->sample_frequency;
 self->transposition=(gp_CentsToRatio(transposition)-speed)*(grain_size>=1.0f?1000.0f/grain_size:1.0f);
 Aud_Phasor_SetFreq(&self->phs,fabsf(self->speed/2.0f));Aud_Phasor_SetFreq(&self->phs2,fabsf(self->speed/2.0f));
 Aud_Phasor_SetFreq(&self->phsImp,fabsf(self->transposition));Aud_Phasor_SetFreq(&self->phsImp2,fabsf(self->transposition));
 self->idxSpeed=gp_NegativeInvert(&self->phs,self->speed)*(float)self->size;
 self->idxSpeed2=gp_NegativeInvert(&self->phs2,self->speed)*(float)self->size;
 self->idxTransp=gp_NegativeInvert(&self->phsImp,self->transposition)*(grain_size*0.001f*self->sample_rate);
 self->idxTransp2=gp_NegativeInvert(&self->phsImp2,self->transposition)*(grain_size*0.001f*self->sample_rate);
 self->idx=gp_WrapIdx((uint32_t)(self->idxSpeed+self->idxTransp),self->size);
 self->idx2=gp_WrapIdx((uint32_t)(self->idxSpeed2+self->idxTransp2),self->size);
 self->sig=self->sample[self->idx]*self->cosEnv[(uint32_t)(Aud_Phasor_Process(&self->phs)*256.0f)];
 self->sig2=self->sample[self->idx2]*self->cosEnv[(uint32_t)(Aud_Phasor_Process(&self->phs2)*256.0f)];
 return (self->sig+self->sig2)/2.0f;}
