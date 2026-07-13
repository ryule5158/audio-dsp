#include "aud_synthsnaredrum.h"
#include "aud_dsp.h"
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
static float ssd_DistortedSine(float phase){float tri=(phase<0.5f?phase:1.0f-phase)*4.0f-1.3f;return 2.0f*tri/(1.0f+fabsf(tri));}
static bool ssd_even=true;
void Aud_SyntheticSnareDrum_Init(Aud_SyntheticSnareDrum *self,float sr){if(self==NULL)return;self->sample_rate=sr;
 self->phase[0]=self->phase[1]=0.0f;self->drum_amplitude=self->snare_amplitude=self->fm=0.0f;self->hold_counter=0;self->sustain_gain=0.0f;
 self->sustain=false;self->accent=0.6f;self->f0=200.0f/sr;self->fm_amount=0.01f;self->fm_amount_sq=0.01f;self->decay=0.3f;self->snappy=0.7f;self->trig=false;
 Aud_Svf_Init(&self->drum_lp,sr);Aud_Svf_Init(&self->snare_hp,sr);Aud_Svf_Init(&self->snare_lp,sr);}
float Aud_SyntheticSnareDrum_Process(Aud_SyntheticSnareDrum *self,bool trigger){if(self==NULL)return 0.0f;
 float dxt=self->decay*(1.0f+self->decay*(self->decay-1.0f));
 float dd=1.0f-1.0f/(0.015f*self->sample_rate)*powf(2.0f,(1.0f/12.0f)*(-dxt*72.0f-self->fm_amount*12.0f+self->snappy*7.0f));
 float sd=1.0f-1.0f/(0.01f*self->sample_rate)*powf(2.0f,(1.0f/12.0f)*(-self->decay*60.0f-self->snappy*7.0f));
 float fmd=1.0f-1.0f/(0.007f*self->sample_rate);
 float snappy=aud_fclamp(self->snappy*1.1f-0.05f,0.0f,1.0f);
 float dl=sqrtf(1.0f-snappy),sl=sqrtf(snappy);
 float snfmin=aud_fmin(10.0f*self->f0,0.5f),snfmax=aud_fmin(35.0f*self->f0,0.5f);
 Aud_Svf_SetFreq(&self->snare_hp,snfmin*self->sample_rate);Aud_Svf_SetFreq(&self->snare_lp,snfmax*self->sample_rate);
 Aud_Svf_SetRes(&self->snare_lp,0.5f+2.0f*snappy);Aud_Svf_SetFreq(&self->drum_lp,3.0f*self->f0*self->sample_rate);
 if(trigger||self->trig){self->trig=false;self->snare_amplitude=self->drum_amplitude=0.3f+0.7f*self->accent;self->fm=1.0f;
  self->phase[0]=self->phase[1]=0.0f;self->hold_counter=(int)((0.04f+self->decay*0.03f)*self->sample_rate);}
 ssd_even=!ssd_even;
 if(self->sustain){self->sustain_gain=self->snare_amplitude=self->accent*self->decay;self->drum_amplitude=self->snare_amplitude;self->fm=0.0f;}
 else{self->drum_amplitude*=(self->drum_amplitude>0.03f||ssd_even)?dd:1.0f;
  if(self->hold_counter)--self->hold_counter;else self->snare_amplitude*=sd;self->fm*=fmd;}
 float rn=0.0f,rna=(0.125f-self->f0)*8.0f;rna=aud_fclamp(rna,0.0f,1.0f);rna*=rna;rna*=self->fm_amount;
 rn+=self->phase[0]>0.5f?-1.0f:1.0f;rn+=self->phase[1]>0.5f?-1.0f:1.0f;rn*=rna*0.025f;
 float f=self->f0*(1.0f+self->fm_amount*(4.0f*self->fm));self->phase[0]+=f;self->phase[1]+=f*1.47f;
 if(rna>0.1f){if(self->phase[0]>=1.0f+rn)self->phase[0]=1.0f-self->phase[0];if(self->phase[1]>=1.0f+rn)self->phase[1]=1.0f-self->phase[1];}
 else{if(self->phase[0]>=1.0f)self->phase[0]-=1.0f;if(self->phase[1]>=1.0f)self->phase[1]-=1.0f;}
 float drum=-0.1f;drum+=ssd_DistortedSine(self->phase[0])*0.60f;drum+=ssd_DistortedSine(self->phase[1])*0.25f;drum*=self->drum_amplitude*dl;
 Aud_Svf_Process(&self->drum_lp,drum);drum=Aud_Svf_Low(&self->drum_lp);
 float noise=(float)rand()/(float)RAND_MAX;Aud_Svf_Process(&self->snare_lp,noise);float snare=Aud_Svf_Low(&self->snare_lp);
 Aud_Svf_Process(&self->snare_hp,snare);snare=Aud_Svf_High(&self->snare_hp);snare=(snare+0.1f)*(self->snare_amplitude+self->fm)*sl;
 return snare+drum;}
void Aud_SyntheticSnareDrum_Trig(Aud_SyntheticSnareDrum *self){if(self)self->trig=true;}
void Aud_SyntheticSnareDrum_SetSustain(Aud_SyntheticSnareDrum *self,bool s){if(self)self->sustain=s;}
void Aud_SyntheticSnareDrum_SetAccent(Aud_SyntheticSnareDrum *self,float a){if(self)self->accent=aud_fclamp(a,0.0f,1.0f);}
void Aud_SyntheticSnareDrum_SetFreq(Aud_SyntheticSnareDrum *self,float f){if(self){f/=self->sample_rate;self->f0=aud_fclamp(f,0.0f,1.0f);}}
void Aud_SyntheticSnareDrum_SetFmAmount(Aud_SyntheticSnareDrum *self,float a){if(self){self->fm_amount=aud_fclamp(a,0.0f,1.0f);self->fm_amount_sq=self->fm_amount*self->fm_amount;}}
void Aud_SyntheticSnareDrum_SetDecay(Aud_SyntheticSnareDrum *self,float d){if(self)self->decay=aud_fmax(d,0.0f);}
void Aud_SyntheticSnareDrum_SetSnappy(Aud_SyntheticSnareDrum *self,float s){if(self)self->snappy=aud_fclamp(s,0.0f,1.0f);}
