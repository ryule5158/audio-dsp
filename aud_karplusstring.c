#include "aud_karplusstring.h"
#include "aud_dsp.h"
#include <math.h>
#include <string.h>
#include <stddef.h>
void Aud_String_Init(Aud_String *self,float sr){if(self==NULL)return;self->sample_rate=sr;self->frequency=0.02f;self->brightness=0.5f;
 self->damping=0.5f;self->nonlinearity=0.0f;Aud_DelayLine_Init(&self->delay,self->del_buf,AUD_STRING_MAX_DELAY);
 Aud_DcBlock_Init(&self->dc,sr);Aud_OnePole_Init(&self->filter);Aud_OnePole_SetFrequency(&self->filter,0.1f);}
void Aud_String_Reset(Aud_String *self){if(self==NULL)return;Aud_DelayLine_Reset(&self->delay);}
void Aud_String_SetFreq(Aud_String *self,float f){if(self){self->frequency=f/self->sample_rate;if(self->frequency>0.5f)self->frequency=0.5f;}}
void Aud_String_SetBrightness(Aud_String *self,float b){if(self)self->brightness=aud_fclamp(b,0.0f,1.0f);}
void Aud_String_SetDamping(Aud_String *self,float d){if(self)self->damping=aud_fclamp(d,0.0f,1.0f);}
void Aud_String_SetNonLinearity(Aud_String *self,float nl){if(self)self->nonlinearity=nl;}
float Aud_String_Process(Aud_String *self,float in){if(self==NULL)return in;
 float delay_samples=self->sample_rate/self->frequency;if(delay_samples>AUD_STRING_MAX_DELAY-2.0f)delay_samples=AUD_STRING_MAX_DELAY-2.0f;
 Aud_DelayLine_SetDelay(&self->delay,delay_samples);float rd=Aud_DelayLine_Read(&self->delay);
 float nl=rd*self->nonlinearity;rd+=nl*nl*nl;float damp=self->damping*rd+(1.0f-self->damping)*in;
 Aud_DelayLine_Write(&self->delay,damp);Aud_OnePole_SetFrequency(&self->filter,0.1f+0.4f*self->brightness);
 return Aud_OnePole_Process(&self->filter,rd);}
