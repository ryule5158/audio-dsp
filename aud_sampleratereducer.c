#include "aud_sampleratereducer.h"
#include "aud_dsp.h"
#include <stddef.h>
void Aud_SampleRateReducer_Init(Aud_SampleRateReducer *self){
 if(self==NULL)return;self->frequency=.2f;self->phase=self->sample=self->next_sample=self->previous_sample=0.0f;}
float Aud_SampleRateReducer_Process(Aud_SampleRateReducer *self,float in){
 if(self==NULL)return in;float ts=self->next_sample;self->next_sample=0.0f;self->phase+=self->frequency;
 if(self->phase>=1.0f){self->phase-=1.0f;float t=self->phase/self->frequency;
  float ns=self->previous_sample+(in-self->previous_sample)*(1.0f-t);
  float disc=ns-self->sample;ts+=disc*aud_this_blep(t);
  self->next_sample=disc*aud_next_blep(t);self->sample=ns;}
 self->next_sample+=self->sample;self->previous_sample=in;return ts;}
void Aud_SampleRateReducer_SetFreq(Aud_SampleRateReducer *self,float f){if(self)self->frequency=aud_fclamp(f,0.0f,1.0f);}
