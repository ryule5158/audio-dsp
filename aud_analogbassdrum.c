#include "aud_analogbassdrum.h"
#include "aud_dsp.h"
#include <math.h>
#include <stddef.h>
static float abd_Diode(float x){if(x>=0.0f)return x;x*=2.0f;return 0.7f*x/(1.0f+fabsf(x));}
void Aud_AnalogBassDrum_Init(Aud_AnalogBassDrum *self,float sr){if(self==NULL)return;self->sample_rate=sr;self->trig=false;
 self->pulse_remaining_samples=self->fm_pulse_remaining_samples=0;self->pulse=self->pulse_height=self->pulse_lp=self->fm_pulse_lp=0.0f;
 self->retrig_pulse=self->lp_out=self->tone_lp=self->sustain_gain=self->phase=0.0f;
 self->sustain=false;self->accent=0.1f;self->f0=50.0f/sr;self->tone=0.1f;self->decay=-0.07f;self->self_fm_amount=50.0f;self->attack_fm_amount=25.0f;
 Aud_Svf_Init(&self->resonator,sr);}
float Aud_AnalogBassDrum_Process(Aud_AnalogBassDrum *self,bool trigger){if(self==NULL)return 0.0f;
 int kTPD=(int)(1.0e-3f*self->sample_rate),kFMPD=(int)(6.0e-3f*self->sample_rate);
 float kPDT=0.2e-3f*self->sample_rate,kPFT=0.1e-3f*self->sample_rate,kRPD=0.05f*self->sample_rate;
 float scale=0.001f/self->f0,q=1500.0f*powf(2.0f,(1.0f/12.0f)*self->decay*80.0f);
 float tone_f=aud_fmin(4.0f*self->f0*powf(2.0f,(1.0f/12.0f)*self->tone*108.0f),1.0f);
 float exciter_leak=0.08f*(self->tone+0.25f);
 if(trigger||self->trig){self->trig=false;self->pulse_remaining_samples=kTPD;self->fm_pulse_remaining_samples=kFMPD;
  self->pulse_height=3.0f+7.0f*self->accent;self->lp_out=0.0f;}
 float pulse=0.0f;
 if(self->pulse_remaining_samples){--self->pulse_remaining_samples;pulse=self->pulse_remaining_samples?self->pulse_height:self->pulse_height-1.0f;self->pulse=pulse;}
 else{self->pulse*=1.0f-1.0f/kPDT;pulse=self->pulse;}
 if(self->sustain)pulse=0.0f;
 aud_fonepole(&self->pulse_lp,pulse,1.0f/kPFT);pulse=abd_Diode((pulse-self->pulse_lp)+pulse*0.044f);
 float fm_pulse=0.0f;
 if(self->fm_pulse_remaining_samples){--self->fm_pulse_remaining_samples;fm_pulse=1.0f;self->retrig_pulse=self->fm_pulse_remaining_samples?0.0f:-0.8f;}
 else{self->retrig_pulse*=1.0f-1.0f/kRPD;}
 if(self->sustain)fm_pulse=0.0f;
 aud_fonepole(&self->fm_pulse_lp,fm_pulse,1.0f/kPFT);
 float punch=0.7f+abd_Diode(10.0f*self->lp_out-1.0f);
 float afm=self->fm_pulse_lp*1.7f*self->attack_fm_amount,sfm=punch*0.08f*self->self_fm_amount;
 float f=self->f0*(1.0f+afm+sfm);f=aud_fclamp(f,0.0f,0.4f);
 float ro;
 if(self->sustain){self->sustain_gain=self->accent*self->decay;self->phase+=f;if(self->phase>=1.0f)self->phase-=1.0f;
  ro=sinf(AUD_TWOPI*self->phase)*self->sustain_gain;self->lp_out=cosf(AUD_TWOPI*self->phase)*self->sustain_gain;}
 else{Aud_Svf_SetFreq(&self->resonator,f*self->sample_rate);Aud_Svf_SetRes(&self->resonator,0.4f*q*f);
  Aud_Svf_Process(&self->resonator,(pulse-self->retrig_pulse*0.2f)*scale);ro=Aud_Svf_Band(&self->resonator);self->lp_out=Aud_Svf_Low(&self->resonator);}
 aud_fonepole(&self->tone_lp,pulse*exciter_leak+ro,tone_f);return self->tone_lp;}
void Aud_AnalogBassDrum_Trig(Aud_AnalogBassDrum *self){if(self)self->trig=true;}
void Aud_AnalogBassDrum_SetSustain(Aud_AnalogBassDrum *self,bool s){if(self)self->sustain=s;}
void Aud_AnalogBassDrum_SetAccent(Aud_AnalogBassDrum *self,float a){if(self)self->accent=aud_fclamp(a,0.0f,1.0f);}
void Aud_AnalogBassDrum_SetFreq(Aud_AnalogBassDrum *self,float f){if(self){f/=self->sample_rate;self->f0=aud_fclamp(f,0.0f,0.5f);}}
void Aud_AnalogBassDrum_SetTone(Aud_AnalogBassDrum *self,float t){if(self)self->tone=aud_fclamp(t,0.0f,1.0f);}
void Aud_AnalogBassDrum_SetDecay(Aud_AnalogBassDrum *self,float d){if(self){self->decay=d*0.1f;self->decay-=0.1f;}}
void Aud_AnalogBassDrum_SetAttackFmAmount(Aud_AnalogBassDrum *self,float a){if(self)self->attack_fm_amount=a*50.0f;}
void Aud_AnalogBassDrum_SetSelfFmAmount(Aud_AnalogBassDrum *self,float a){if(self)self->self_fm_amount=a*50.0f;}
