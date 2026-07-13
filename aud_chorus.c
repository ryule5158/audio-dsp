#include "aud_chorus.h"
#include "aud_dsp.h"
#include <stddef.h>

static float chorus_ProcessLfo(Aud_ChorusEngine *self){
 if(self==NULL)return 0.0f;self->lfo_phase+=self->lfo_freq;
 if(self->lfo_phase>1.0f){self->lfo_phase=1.0f-(self->lfo_phase-1.0f);self->lfo_freq*=-1.0f;}
 else if(self->lfo_phase<-1.0f){self->lfo_phase=-1.0f-(self->lfo_phase+1.0f);self->lfo_freq*=-1.0f;}
 return self->lfo_phase*self->lfo_amp;}

void Aud_ChorusEngine_Init(Aud_ChorusEngine *self,float sr){if(self==NULL)return;self->sr=sr;
 Aud_DelayLine_Init(&self->del,self->del_buf,AUD_CHORUS_DELAY_LEN);self->lfo_amp=0.0f;self->feedback=0.2f;
 Aud_ChorusEngine_SetDelay(self,0.75f);self->lfo_phase=0.0f;self->lfo_freq=0.025f;self->lfo_amp=0.9f;}

float Aud_ChorusEngine_Process(Aud_ChorusEngine *self,float in){
 if(self==NULL)return in;float lfo=chorus_ProcessLfo(self);
 Aud_DelayLine_SetDelay(&self->del,lfo+self->delay);float out=Aud_DelayLine_Read(&self->del);
 Aud_DelayLine_Write(&self->del,in+out*self->feedback);return (in+out)*0.5f;}

void Aud_ChorusEngine_SetLfoDepth(Aud_ChorusEngine *self,float d){if(self){d=aud_fclamp(d,0.0f,0.93f);self->lfo_amp=d*self->delay;}}
void Aud_ChorusEngine_SetLfoFreq(Aud_ChorusEngine *self,float f){if(self){f=4.0f*f/self->sr;f*=(self->lfo_freq<0.0f)?-1.0f:1.0f;self->lfo_freq=aud_fclamp(f,-0.25f,0.25f);}}
void Aud_ChorusEngine_SetDelay(Aud_ChorusEngine *self,float d){if(self)Aud_ChorusEngine_SetDelayMs(self,0.1f+d*7.9f);}
void Aud_ChorusEngine_SetDelayMs(Aud_ChorusEngine *self,float ms){if(self){ms=aud_fmax(0.1f,ms);self->delay=ms*0.001f*self->sr;self->lfo_amp=aud_fmin(self->lfo_amp,self->delay);}}
void Aud_ChorusEngine_SetFeedback(Aud_ChorusEngine *self,float fb){if(self)self->feedback=aud_fclamp(fb,0.0f,1.0f);}

void Aud_Chorus_Init(Aud_Chorus *self,float sr){if(self==NULL)return;
 Aud_ChorusEngine_Init(&self->engines[0],sr);Aud_ChorusEngine_Init(&self->engines[1],sr);
 self->pan[0]=0.25f;self->pan[1]=0.75f;self->gain_frac=0.5f;self->sigl=self->sigr=0.0f;}

float Aud_Chorus_Process(Aud_Chorus *self,float in){if(self==NULL)return in;self->sigl=self->sigr=0.0f;
 for(int i=0;i<2;i++){float sig=Aud_ChorusEngine_Process(&self->engines[i],in);
  self->sigl+=(1.0f-self->pan[i])*sig;self->sigr+=self->pan[i]*sig;}
 self->sigl*=self->gain_frac;self->sigr*=self->gain_frac;return self->sigl;}

float Aud_Chorus_GetLeft(Aud_Chorus *self){return(self!=NULL)?self->sigl:0.0f;}
float Aud_Chorus_GetRight(Aud_Chorus *self){return(self!=NULL)?self->sigr:0.0f;}
void Aud_Chorus_SetPan(Aud_Chorus *self,float l,float r){if(self){self->pan[0]=aud_fclamp(l,0.0f,1.0f);self->pan[1]=aud_fclamp(r,0.0f,1.0f);}}
void Aud_Chorus_SetLfoDepth(Aud_Chorus *self,float l,float r){if(self){Aud_ChorusEngine_SetLfoDepth(&self->engines[0],l);Aud_ChorusEngine_SetLfoDepth(&self->engines[1],r);}}
void Aud_Chorus_SetLfoFreq(Aud_Chorus *self,float l,float r){if(self){Aud_ChorusEngine_SetLfoFreq(&self->engines[0],l);Aud_ChorusEngine_SetLfoFreq(&self->engines[1],r);}}
void Aud_Chorus_SetDelay(Aud_Chorus *self,float l,float r){if(self){Aud_ChorusEngine_SetDelay(&self->engines[0],l);Aud_ChorusEngine_SetDelay(&self->engines[1],r);}}
void Aud_Chorus_SetDelayMs(Aud_Chorus *self,float l,float r){if(self){Aud_ChorusEngine_SetDelayMs(&self->engines[0],l);Aud_ChorusEngine_SetDelayMs(&self->engines[1],r);}}
void Aud_Chorus_SetFeedback(Aud_Chorus *self,float l,float r){if(self){Aud_ChorusEngine_SetFeedback(&self->engines[0],l);Aud_ChorusEngine_SetFeedback(&self->engines[1],r);}}
