#include "aud_analogsnaredrum.h"
#include "aud_dsp.h"
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#define ASD_NUM_MODES 5
void Aud_AnalogSnareDrum_Init(Aud_AnalogSnareDrum *self,float sr){if(self==NULL)return;self->sample_rate=sr;self->trig=false;
 self->pulse_remaining_samples=0;self->pulse=self->pulse_height=self->pulse_lp=self->noise_envelope=self->sustain_gain=0.0f;
 self->sustain=false;self->accent=0.6f;self->f0=200.0f/sr;self->decay=0.3f;self->snappy=0.7f;self->tone=1.0f;
 for(int i=0;i<ASD_NUM_MODES;i++){Aud_Svf_Init(&self->resonator[i],sr);self->phase[i]=0.0f;}Aud_Svf_Init(&self->noise_filter,sr);}
void Aud_AnalogSnareDrum_Trig(Aud_AnalogSnareDrum *self){if(self)self->trig=true;}
void Aud_AnalogSnareDrum_SetSustain(Aud_AnalogSnareDrum *self,bool s){if(self)self->sustain=s;}
void Aud_AnalogSnareDrum_SetAccent(Aud_AnalogSnareDrum *self,float a){if(self)self->accent=aud_fclamp(a,0.0f,1.0f);}
void Aud_AnalogSnareDrum_SetFreq(Aud_AnalogSnareDrum *self,float f){if(self){f/=self->sample_rate;self->f0=aud_fclamp(f,0.0f,0.4f);}}
void Aud_AnalogSnareDrum_SetTone(Aud_AnalogSnareDrum *self,float t){if(self){self->tone=aud_fclamp(t,0.0f,1.0f)*2.0f;}}
void Aud_AnalogSnareDrum_SetDecay(Aud_AnalogSnareDrum *self,float d){if(self)self->decay=d;}
void Aud_AnalogSnareDrum_SetSnappy(Aud_AnalogSnareDrum *self,float s){if(self)self->snappy=aud_fclamp(s,0.0f,1.0f);}
float Aud_AnalogSnareDrum_Process(Aud_AnalogSnareDrum *self,bool trigger){if(self==NULL)return 0.0f;
 float dxt=self->decay*(1.0f+self->decay*(self->decay-1.0f));int kTPD=(int)(1.0e-3f*self->sample_rate);float kPDT=0.1e-3f*self->sample_rate;
 float q=2000.0f*powf(2.0f,(1.0f/12.0f)*dxt*84.0f);
 float ned=1.0f-0.0017f*powf(2.0f,(1.0f/12.0f)*(-self->decay*(50.0f+self->snappy*10.0f)));
 float el=self->snappy*(2.0f-self->snappy)*0.1f;
 float snappy=aud_fclamp(self->snappy*1.1f-0.05f,0.0f,1.0f),tone=self->tone;
 if(trigger||self->trig){self->trig=false;self->pulse_remaining_samples=kTPD;self->pulse_height=3.0f+7.0f*self->accent;self->noise_envelope=2.0f;}
 static const float kMF[ASD_NUM_MODES]={1.00f,2.00f,3.18f,4.16f,5.62f};
 float f[ASD_NUM_MODES],gain[ASD_NUM_MODES];
 for(int i=0;i<ASD_NUM_MODES;i++){f[i]=aud_fmin(self->f0*kMF[i],0.499f);Aud_Svf_SetFreq(&self->resonator[i],f[i]*self->sample_rate);
  Aud_Svf_SetRes(&self->resonator[i],(f[i]*(i==0?q:q*0.25f))*0.2f);}
 if(tone<0.666667f){tone*=1.5f;gain[0]=1.5f+(1.0f-tone)*(1.0f-tone)*4.5f;gain[1]=2.0f*tone+0.15f;
  for(int i=2;i<ASD_NUM_MODES;i++)gain[i]=0.0f;}
 else{tone=(tone-0.666667f)*3.0f;gain[0]=1.5f-tone*0.5f;gain[1]=2.15f-tone*0.7f;
  for(int i=2;i<ASD_NUM_MODES;i++){gain[i]=tone;tone*=tone;}}
 float f_noise=aud_fclamp(self->f0*16.0f,0.0f,0.499f);Aud_Svf_SetFreq(&self->noise_filter,f_noise*self->sample_rate);
 Aud_Svf_SetRes(&self->noise_filter,f_noise*1.5f);
 float pulse=0.0f;
 if(self->pulse_remaining_samples){--self->pulse_remaining_samples;pulse=self->pulse_remaining_samples?self->pulse_height:self->pulse_height-1.0f;self->pulse=pulse;}
 else{self->pulse*=1.0f-1.0f/kPDT;pulse=self->pulse;}
 float sgv=self->sustain_gain=self->accent*self->decay;
 self->pulse_lp=aud_fclamp(self->pulse_lp,pulse,0.75f);
 float shell=0.0f;
 for(int i=0;i<ASD_NUM_MODES;i++){float exc=(i==0)?((pulse-self->pulse_lp)+0.006f*pulse):(0.026f*pulse);
  self->phase[i]+=f[i];if(self->phase[i]>=1.0f)self->phase[i]-=1.0f;
  Aud_Svf_Process(&self->resonator[i],exc);
  shell+=gain[i]*(self->sustain?sinf(self->phase[i]*AUD_TWOPI)*sgv*0.25f:Aud_Svf_Band(&self->resonator[i])+exc*el);}
 shell=aud_soft_clip(shell);
 float noise=2.0f*((float)rand()/(float)RAND_MAX)-1.0f;if(noise<0.0f)noise=0.0f;
 self->noise_envelope*=ned;noise*=(self->sustain?sgv:self->noise_envelope)*snappy*2.0f;
 Aud_Svf_Process(&self->noise_filter,noise);noise=Aud_Svf_Band(&self->noise_filter);
 return noise+shell*(1.0f-snappy);}
