#include "aud_oscillatorbank.h"
#include "aud_dsp.h"
#include <math.h>
#include <stddef.h>
static bool ob_cmp(float a,float b){return fabsf(a-b)>0.0000001f;}
void Aud_OscillatorBank_Init(Aud_OscillatorBank *self,float sr){if(self==NULL)return;self->sample_rate=sr;self->phase=self->next_sample=0.0f;
 self->segment=0;self->frequency=0.0f;self->saw_8_gain=self->saw_4_gain=self->saw_2_gain=self->saw_1_gain=0.0f;
 self->recalc=self->recalc_gain=true;self->gain=1.0f;int i;for(i=0;i<7;i++)self->registration[i]=self->unshifted_registration[i]=0.0f;
 self->unshifted_registration[0]=1.0f;self->frequency=440.0f/8.0f/sr;self->recalc=true;self->recalc_gain=true;}
float Aud_OscillatorBank_Process(Aud_OscillatorBank *self){if(self==NULL)return 0.0f;
 if(self->recalc){self->recalc=false;self->frequency*=8.0f;size_t shift=0;
  while(self->frequency>0.5f){shift+=2;self->frequency*=0.5f;}
  for(int i=0;i<7;i++)self->registration[i]=0.0f;
  for(size_t i=0;i<7-shift;i++)self->registration[i+shift]=self->unshifted_registration[i];}
 if(self->recalc_gain||self->recalc){self->saw_8_gain=(self->registration[0]+2.0f*self->registration[1])*self->gain;
  self->saw_4_gain=(self->registration[2]-self->registration[1]+2.0f*self->registration[3])*self->gain;
  self->saw_2_gain=(self->registration[4]-self->registration[3]+2.0f*self->registration[5])*self->gain;
  self->saw_1_gain=(self->registration[6]-self->registration[5])*self->gain;}
 float ts=self->next_sample;self->next_sample=0.0f;self->phase+=self->frequency;
 int ns=(int)self->phase;if(ns!=self->segment){float d=0.0f;
  if(ns==8){self->phase-=8.0f;ns-=8;d-=self->saw_8_gain;}
  if((ns&3)==0)d-=self->saw_4_gain;if((ns&1)==0)d-=self->saw_2_gain;d-=self->saw_1_gain;
  if(d!=0.0f){float frac=self->phase-(float)ns,t=frac/self->frequency;ts+=aud_this_blep(t)*d;self->next_sample+=aud_next_blep(t)*d;}}
 self->segment=ns;self->next_sample+=(self->phase-4.0f)*self->saw_8_gain*0.125f;
 self->next_sample+=(self->phase-(float)(self->segment&4)-2.0f)*self->saw_4_gain*0.25f;
 self->next_sample+=(self->phase-(float)(self->segment&6)-1.0f)*self->saw_2_gain*0.5f;
 self->next_sample+=(self->phase-(float)(self->segment&7)-0.5f)*self->saw_1_gain;return 2.0f*ts;}
void Aud_OscillatorBank_SetFreq(Aud_OscillatorBank *self,float f){if(self==NULL)return;
 f/=self->sample_rate;f=(f>0.5f)?0.5f:f;self->recalc=ob_cmp(f,self->frequency)||self->recalc;self->frequency=f;}
void Aud_OscillatorBank_SetAmplitudes(Aud_OscillatorBank *self,const float*amps){if(self==NULL||amps==NULL)return;
 for(int i=0;i<7;i++){self->recalc=ob_cmp(self->unshifted_registration[i],amps[i])||self->recalc;self->unshifted_registration[i]=amps[i];}}
void Aud_OscillatorBank_SetSingleAmp(Aud_OscillatorBank *self,float amp,int idx){if(self==NULL||idx<0||idx>6)return;
 self->recalc=ob_cmp(self->unshifted_registration[idx],amp)||self->recalc;self->unshifted_registration[idx]=amp;}
void Aud_OscillatorBank_SetGain(Aud_OscillatorBank *self,float g){if(self==NULL)return;
 g=(g>1.0f)?1.0f:((g<0.0f)?0.0f:g);self->recalc_gain=ob_cmp(g,self->gain)||self->recalc_gain;self->gain=g;}
