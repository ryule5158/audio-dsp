#include "aud_resonator.h"
#include "aud_dsp.h"
#include <math.h>
#include <stddef.h>
#define BS AUD_RESONATOR_BATCH_SIZE
static float rs_NthHarmonicCompensation(int n,float stiffness){float sf=1.0f;int i;for(i=0;i<n-1;i++){sf+=stiffness;stiffness*=(stiffness<0.0f?0.93f:0.98f);}return 1.0f/sf;}
static float rs_fasttan(float f){float f2=f*f;return f*(AUD_PI+f2*(3.260e-01f*AUD_PI*AUD_PI*AUD_PI+f2*1.823e-01f*AUD_PI*AUD_PI*AUD_PI*AUD_PI*AUD_PI));}
static float rs_CalcStiff(float sig){if(sig<0.25f){sig=0.25f-sig;sig=-sig*0.25f;}else if(sig<0.3f)sig=0.0f;
 else if(sig<0.9f){sig-=0.3f;sig*=(1.0f/0.6f);}else{sig-=0.9f;sig*=10.0f;sig*=sig;sig=1.5f-cosf(sig*AUD_PI)*0.5f;}return sig;}
void Aud_ResonatorSvf_Init(Aud_ResonatorSvf *self){if(self==NULL)return;int i;for(i=0;i<BS;i++)self->state_1[i]=self->state_2[i]=0.0f;}
void Aud_Resonator_Init(Aud_Resonator *self,float pos,int res,float sr){if(self==NULL)return;self->sample_rate=sr;self->resolution=res<AUD_RESONATOR_MAX_MODES?res:AUD_RESONATOR_MAX_MODES;
 self->frequency=440.0f/sr;self->structure=0.5f;self->brightness=0.5f;self->damping=0.5f;int i;
 for(i=0;i<self->resolution;i++)self->mode_amplitude[i]=cosf(pos*AUD_TWOPI)*0.25f;
 for(i=0;i<AUD_RESONATOR_MAX_MODES/BS;i++)Aud_ResonatorSvf_Init(&self->mode_filters[i]);}
float Aud_Resonator_Process(Aud_Resonator *self,float in){if(self==NULL)return 0.0f;float out=0.0f;
 float stiffness=rs_CalcStiff(self->structure),f0=self->frequency*rs_NthHarmonicCompensation(3,stiffness),brightness=self->brightness;
 float harmonic=f0,sf2=1.0f,input=self->damping*79.7f,q_sqrt=powf(2.0f,input*(1.0f/12.0f)),q=500.0f*q_sqrt*q_sqrt;
 brightness*=1.0f-self->structure*0.3f;brightness*=1.0f-self->damping*0.3f;float q_loss=brightness*(2.0f-brightness)*0.85f+0.15f;
 float mf[BS],mq[BS],ma[BS];int bc=0,bi=0;
 for(int i=0;i<self->resolution;i++){float mfreq=harmonic*sf2;if(mfreq>=0.499f)mfreq=0.499f;float matn=1.0f-mfreq*2.0f;
  mf[bc]=mfreq;mq[bc]=1.0f+mfreq*q;ma[bc]=self->mode_amplitude[i]*matn;bc++;
  if(bc==BS){Aud_ResonatorSvf *svf=&self->mode_filters[bi];float g[BS],r[BS],rpg[BS],h[BS],s1[BS],s2[BS],gs[BS];
   for(int j=0;j<BS;j++){g[j]=rs_fasttan(mf[j]);r[j]=1.0f/mq[j];h[j]=1.0f/(1.0f+r[j]*g[j]+g[j]*g[j]);rpg[j]=r[j]+g[j];s1[j]=svf->state_1[j];s2[j]=svf->state_2[j];gs[j]=ma[j];}
   float sin=in,sout=0.0f;
   for(int j=0;j<BS;j++){float hp=(sin-rpg[j]*s1[j]-s2[j])*h[j],bp=g[j]*hp+s1[j];s1[j]=g[j]*hp+bp;
    float lp=g[j]*bp+s2[j];s2[j]=g[j]*bp+lp;sout+=gs[j]*bp;}
   out+=sout;for(int j=0;j<BS;j++){svf->state_1[j]=s1[j];svf->state_2[j]=s2[j];}bc=0;bi++;}
  sf2+=stiffness;stiffness*=(stiffness<0.0f?0.93f:0.98f);harmonic+=f0;q*=q_loss;}return out;}
void Aud_Resonator_SetFreq(Aud_Resonator *self,float f){if(self)self->frequency=f/self->sample_rate;}
void Aud_Resonator_SetStructure(Aud_Resonator *self,float s){if(self)self->structure=aud_fclamp(s,0.0f,1.0f);}
void Aud_Resonator_SetBrightness(Aud_Resonator *self,float b){if(self)self->brightness=aud_fclamp(b,0.0f,1.0f);}
void Aud_Resonator_SetDamping(Aud_Resonator *self,float d){if(self)self->damping=aud_fclamp(d,0.0f,1.0f);}
