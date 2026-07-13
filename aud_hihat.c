#include "aud_hihat.h"
#include "aud_dsp.h"
#include "aud_osc.h"
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
static float hh_SemitonesToRatio(float in){return powf(2.0f,in*(1.0f/12.0f));}
static float hh_SwingVCA(float s,float gain){s*=(s>0.0f)?10.0f:0.1f;s=s/(1.0f+fabsf(s));return(s+1.0f)*gain;}
static float hh_LinearVCA(float s,float gain){return s*gain;}
void Aud_SquareNoise_Init(Aud_SquareNoise *self,float sr){(void)sr;if(self==NULL)return;for(int i=0;i<6;i++)self->phase[i]=0;}
float Aud_SquareNoise_Process(Aud_SquareNoise *self,float f0){if(self==NULL)return 0.0f;
 const float ratios[6]={1.0f,1.304f,1.466f,1.787f,1.932f,2.536f};uint32_t inc[6],ph[6];
 for(int i=0;i<6;i++){float f=f0*ratios[i];if(f>=0.499f)f=0.499f;inc[i]=(uint32_t)(f*4294967296.0f);ph[i]=self->phase[i];}
 ph[0]+=inc[0];ph[1]+=inc[1];ph[2]+=inc[2];ph[3]+=inc[3];ph[4]+=inc[4];ph[5]+=inc[5];
 uint32_t noise=0;noise+=(ph[0]>>31);noise+=(ph[1]>>31);noise+=(ph[2]>>31);noise+=(ph[3]>>31);noise+=(ph[4]>>31);noise+=(ph[5]>>31);
 for(int i=0;i<6;i++)self->phase[i]=ph[i];return 0.33f*(float)noise-1.0f;}
void Aud_RingModNoise_Init(Aud_RingModNoise *self,float sr){if(self==NULL)return;self->sample_rate=sr;for(int i=0;i<6;i++)Aud_Osc_Init(&self->oscillator[i],sr);}
static float rmn_ProcessPair(Aud_Osc *osc,float f1,float f2,float sr){Aud_Osc_SetWaveform(&osc[0],AUD_WAVE_SQUARE);Aud_Osc_SetFreq(&osc[0],f1*sr);
 float t1=Aud_Osc_Process(&osc[0]);Aud_Osc_SetWaveform(&osc[1],AUD_WAVE_SAW);Aud_Osc_SetFreq(&osc[1],f2*sr);float t2=Aud_Osc_Process(&osc[1]);return t1*t2;}
float Aud_RingModNoise_Process(Aud_RingModNoise *self,float f0){if(self==NULL)return 0.0f;float r=f0/(0.01f+f0),sr=self->sample_rate;
 float out=rmn_ProcessPair(&self->oscillator[0],200.0f/sr*r,7530.0f/sr*r,sr);out+=rmn_ProcessPair(&self->oscillator[2],510.0f/sr*r,8075.0f/sr*r,sr);
 out+=rmn_ProcessPair(&self->oscillator[4],730.0f/sr*r,10500.0f/sr*r,sr);return out;}
void Aud_HiHat_Init(Aud_HiHat *self,float sr){if(self==NULL)return;self->sample_rate=sr;self->trig=false;
 self->envelope=self->noise_clock=self->noise_sample=self->sustain_gain=0.0f;
 Aud_SquareNoise_Init(&self->metallic_noise,sr);Aud_Svf_Init(&self->noise_coloration_svf,sr);Aud_Svf_Init(&self->hpf,sr);
 self->f0=3000.0f/sr;self->tone=0.5f;self->decay=0.2f;self->noisiness=0.64f;self->accent=0.8f;self->sustain=false;}
float Aud_HiHat_Process(Aud_HiHat *self,bool trigger){if(self==NULL)return 0.0f;
 float ed=1.0f-0.003f*hh_SemitonesToRatio(-self->decay*84.0f),cd=1.0f-0.0025f*hh_SemitonesToRatio(-self->decay*36.0f);
 if(trigger||self->trig){self->trig=false;self->envelope=(1.5f+0.5f*(1.0f-self->decay))*(0.3f+0.7f*self->accent);}
 float out=Aud_SquareNoise_Process(&self->metallic_noise,2.0f*self->f0);
 float cutoff=150.0f/self->sample_rate*hh_SemitonesToRatio(self->tone*72.0f);cutoff=aud_fclamp(cutoff,0.0f,16000.0f/self->sample_rate);
 Aud_Svf_SetFreq(&self->noise_coloration_svf,cutoff*self->sample_rate);Aud_Svf_SetRes(&self->noise_coloration_svf,3.0f+6.0f*self->tone);
 Aud_Svf_Process(&self->noise_coloration_svf,out);out=Aud_Svf_Band(&self->noise_coloration_svf);
 float nf=self->f0*(16.0f+16.0f*(1.0f-self->noisiness));nf=aud_fclamp(nf,0.0f,0.5f);self->noise_clock+=nf;
 if(self->noise_clock>=1.0f){self->noise_clock-=1.0f;self->noise_sample=((float)rand()/(float)RAND_MAX)-0.5f;}
 out+=self->noisiness*(self->noise_sample-out);self->sustain_gain=self->accent*self->decay;
 self->envelope*=(self->envelope>0.5f?ed:cd);out=hh_LinearVCA(out,self->sustain?self->sustain_gain:self->envelope);
 Aud_Svf_SetFreq(&self->hpf,cutoff*self->sample_rate);Aud_Svf_SetRes(&self->hpf,0.5f);Aud_Svf_Process(&self->hpf,out);out=Aud_Svf_High(&self->hpf);return out;}
void Aud_HiHat_Trig(Aud_HiHat *self){if(self)self->trig=true;}
void Aud_HiHat_SetSustain(Aud_HiHat *self,bool s){if(self)self->sustain=s;}
void Aud_HiHat_SetAccent(Aud_HiHat *self,float a){if(self)self->accent=aud_fclamp(a,0.0f,1.0f);}
void Aud_HiHat_SetFreq(Aud_HiHat *self,float f){if(self){f/=self->sample_rate;self->f0=aud_fclamp(f,0.0f,1.0f);}}
void Aud_HiHat_SetTone(Aud_HiHat *self,float t){if(self)self->tone=aud_fclamp(t,0.0f,1.0f);}
void Aud_HiHat_SetDecay(Aud_HiHat *self,float d){if(self){d=aud_fmax(d,0.0f)*1.7f-1.2f;self->decay=d;}}
void Aud_HiHat_SetNoisiness(Aud_HiHat *self,float n){if(self){n=aud_fclamp(n,0.0f,1.0f);self->noisiness=n*n;}}
