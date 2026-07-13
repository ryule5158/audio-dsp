#include "aud_synthbassdrum.h"
#include "aud_dsp.h"
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
/* Click */
void Aud_SyntheticBassDrumClick_Init(Aud_SyntheticBassDrumClick *self,float sr){if(self==NULL)return;self->lp=self->hp=0.0f;
 Aud_Svf_Init(&self->filter,sr);Aud_Svf_SetFreq(&self->filter,5000.0f);Aud_Svf_SetRes(&self->filter,1.0f);}
float Aud_SyntheticBassDrumClick_Process(Aud_SyntheticBassDrumClick *self,float in){if(self==NULL)return 0.0f;
 float err=in-self->lp;self->lp+=(err>0.0f?0.5f:0.1f)*err;aud_fonepole(&self->hp,self->lp,0.04f);
 Aud_Svf_Process(&self->filter,self->lp-self->hp);return Aud_Svf_Low(&self->filter);}
/* AttackNoise */
void Aud_SyntheticBassDrumAttackNoise_Init(Aud_SyntheticBassDrumAttackNoise *self){if(self==NULL)return;self->lp=self->hp=0.0f;}
float Aud_SyntheticBassDrumAttackNoise_Process(Aud_SyntheticBassDrumAttackNoise *self){if(self==NULL)return 0.0f;
 float s=(float)rand()/(float)RAND_MAX;aud_fonepole(&self->lp,s,0.05f);aud_fonepole(&self->hp,self->lp,0.005f);return self->lp-self->hp;}
/* Drum */
static float sbd_DistortedSine(float phase,float pn,float dirt){phase+=pn*dirt;int pi=(int)phase;float pf=phase-(float)pi;
 float tri=(pf<0.5f?pf:1.0f-pf)*4.0f-1.0f;float sine=2.0f*tri/(1.0f+fabsf(tri));float cs=sinf(AUD_TWOPI*(pf+0.75f));return sine+(1.0f-dirt)*(cs-sine);}
static float sbd_TransistorVCA(float s,float g){s=(s-0.6f)*g;return 3.0f*s/(2.0f+fabsf(s))+g*0.3f;}
void Aud_SyntheticBassDrum_Init(Aud_SyntheticBassDrum *self,float sr){if(self==NULL)return;self->sample_rate=sr;self->trig=false;
 self->phase=self->phase_noise=self->f0=0.0f;self->fm=self->fm_lp=self->body_env_lp=self->body_env=self->body_env_pulse_width=0;
 self->fm_pulse_width=0;self->tone_lp=self->sustain_gain=0.0f;
 self->new_f0=100.0f/sr;self->sustain=false;self->accent=0.2f;self->tone=0.6f;self->decay=0.49f;self->dirtiness=0.3f;
 self->fm_envelope_amount=0.6f;self->fm_envelope_decay=0.09f;
 Aud_SyntheticBassDrumClick_Init(&self->click,sr);Aud_SyntheticBassDrumAttackNoise_Init(&self->noise);}
float Aud_SyntheticBassDrum_Process(Aud_SyntheticBassDrum *self,bool trigger){if(self==NULL)return 0.0f;
 float dirt=self->dirtiness*aud_fmax(1.0f-8.0f*self->new_f0,0.0f);
 float fmd=1.0f-1.0f/(0.008f*(1.0f+self->fm_envelope_decay*4.0f)*self->sample_rate);
 float bed=1.0f-1.0f/(0.02f*self->sample_rate)*powf(2.0f,(-self->decay*60.0f)*(1.0f/12.0f));
 float ted=1.0f-1.0f/(0.005f*self->sample_rate);
 float tone_f=aud_fmin(4.0f*self->new_f0*powf(2.0f,(self->tone*108.0f)*(1.0f/12.0f)),1.0f),tl=self->tone;
 if(trigger||self->trig){self->trig=false;self->fm=1.0f;self->body_env=self->transient_env=0.3f+0.7f*self->accent;
  self->body_env_pulse_width=(int)(self->sample_rate*0.001f);self->fm_pulse_width=(int)(self->sample_rate*0.0013f);}
 self->sustain_gain=self->accent*self->decay;
 aud_fonepole(&self->phase_noise,((float)rand()/(float)RAND_MAX)-0.5f,0.002f);float mix=0.0f;
 if(self->sustain){self->f0=self->new_f0;self->phase+=self->f0;if(self->phase>=1.0f)self->phase-=1.0f;
  float body=sbd_DistortedSine(self->phase,self->phase_noise,dirt);mix-=sbd_TransistorVCA(body,self->sustain_gain);}
 else{if(self->fm_pulse_width){--self->fm_pulse_width;self->phase=0.25f;}
  else{self->fm*=fmd;float fm=1.0f+self->fm_envelope_amount*3.5f*self->fm_lp;self->f0=self->new_f0;
   self->phase+=aud_fmin(self->f0*fm,0.5f);if(self->phase>=1.0f)self->phase-=1.0f;}
  if(self->body_env_pulse_width)--self->body_env_pulse_width;else{self->body_env*=bed;self->transient_env*=ted;}
  float elp=0.1f;aud_fonepole(&self->body_env_lp,self->body_env,elp);aud_fonepole(&self->transient_env_lp,self->transient_env,elp);
  aud_fonepole(&self->fm_lp,self->fm,elp);
  float body=sbd_DistortedSine(self->phase,self->phase_noise,dirt);
  float tr=Aud_SyntheticBassDrumClick_Process(&self->click,self->body_env_pulse_width?0.0f:1.0f)+Aud_SyntheticBassDrumAttackNoise_Process(&self->noise);
  mix-=sbd_TransistorVCA(body,self->body_env_lp);mix-=tr*self->transient_env_lp*tl;}
 aud_fonepole(&self->tone_lp,mix,tone_f);return self->tone_lp;}
void Aud_SyntheticBassDrum_Trig(Aud_SyntheticBassDrum *self){if(self)self->trig=true;}
void Aud_SyntheticBassDrum_SetSustain(Aud_SyntheticBassDrum *self,bool s){if(self)self->sustain=s;}
void Aud_SyntheticBassDrum_SetAccent(Aud_SyntheticBassDrum *self,float a){if(self)self->accent=aud_fclamp(a,0.0f,1.0f);}
void Aud_SyntheticBassDrum_SetFreq(Aud_SyntheticBassDrum *self,float f){if(self){f/=self->sample_rate;self->new_f0=aud_fclamp(f,0.0f,1.0f);}}
void Aud_SyntheticBassDrum_SetTone(Aud_SyntheticBassDrum *self,float t){if(self)self->tone=aud_fclamp(t,0.0f,1.0f);}
void Aud_SyntheticBassDrum_SetDecay(Aud_SyntheticBassDrum *self,float d){if(self){d=aud_fclamp(d,0.0f,1.0f);self->decay=d*d;}}
void Aud_SyntheticBassDrum_SetDirtiness(Aud_SyntheticBassDrum *self,float d){if(self)self->dirtiness=aud_fclamp(d,0.0f,1.0f);}
void Aud_SyntheticBassDrum_SetFmEnvelopeAmount(Aud_SyntheticBassDrum *self,float a){if(self)self->fm_envelope_amount=aud_fclamp(a,0.0f,1.0f);}
void Aud_SyntheticBassDrum_SetFmEnvelopeDecay(Aud_SyntheticBassDrum *self,float d){if(self){d=aud_fclamp(d,0.0f,1.0f);self->fm_envelope_decay=d*d;}}
