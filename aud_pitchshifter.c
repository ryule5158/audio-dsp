#include "aud_pitchshifter.h"
#include "aud_dsp.h"
#include <math.h>
#include <stddef.h>
void Aud_PitchShifter_Init(Aud_PitchShifter *self,float sr,float shift){if(self==NULL)return;self->sample_rate=sr;self->shift=shift;
 self->feedback=0.0f;self->mix=0.5f;self->phase=0.0f;self->lfo_freq=0.0f;self->lfo_depth=0.0f;self->enable_lfo=false;
 Aud_DelayLine_Init(&self->delay,self->del_buf,AUD_PITCHSHIFTER_MAX_DELAY);}
float Aud_PitchShifter_Process(Aud_PitchShifter *self,float in){if(self==NULL)return in;
 float ratio=powf(2.0f,self->shift/12.0f);float delay_samples=AUD_PITCHSHIFTER_MAX_DELAY*0.5f;
 float lfo=self->enable_lfo?(sinf(self->phase)*self->lfo_depth):0.0f;self->phase+=self->lfo_freq;
 Aud_DelayLine_SetDelay(&self->delay,delay_samples+lfo);float rd=Aud_DelayLine_Read(&self->delay);
 Aud_DelayLine_Write(&self->delay,in+rd*self->feedback);return in*(1.0f-self->mix)+rd*self->mix;}
void Aud_PitchShifter_SetShift(Aud_PitchShifter *self,float s){if(self)self->shift=s;}
void Aud_PitchShifter_SetFeedback(Aud_PitchShifter *self,float fb){if(self)self->feedback=aud_fclamp(fb,0.0f,0.99f);}
void Aud_PitchShifter_SetMix(Aud_PitchShifter *self,float m){if(self)self->mix=aud_fclamp(m,0.0f,1.0f);}
