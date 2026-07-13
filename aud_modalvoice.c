#include "aud_modalvoice.h"
#include "aud_dsp.h"
#include <math.h>
#include <stddef.h>
void Aud_ModalVoice_Init(Aud_ModalVoice *self,float sr){if(self==NULL)return;self->sample_rate=sr;self->aux=0.0f;
 Aud_ResonatorSvf_Init(&self->excitation_filter);Aud_Resonator_Init(&self->resonator,0.015f,24,sr);
 Aud_Dust_Init(&self->dust);self->sustain=false;self->f0=440.0f/sr;self->accent=0.3f;
 self->structure=0.6f;self->brightness=0.8f;self->damping=0.6f;}
void Aud_ModalVoice_SetSustain(Aud_ModalVoice *self,bool s){if(self)self->sustain=s;}
void Aud_ModalVoice_Trig(Aud_ModalVoice *self){if(self)self->trig=true;}
void Aud_ModalVoice_SetFreq(Aud_ModalVoice *self,float f){if(self){Aud_Resonator_SetFreq(&self->resonator,f);self->f0=f/self->sample_rate;self->f0=aud_fclamp(self->f0,0.0f,0.25f);}}
void Aud_ModalVoice_SetAccent(Aud_ModalVoice *self,float a){if(self)self->accent=aud_fclamp(a,0.0f,1.0f);}
void Aud_ModalVoice_SetStructure(Aud_ModalVoice *self,float s){if(self)Aud_Resonator_SetStructure(&self->resonator,s);}
void Aud_ModalVoice_SetBrightness(Aud_ModalVoice *self,float b){if(self){self->brightness=aud_fclamp(b,0.0f,1.0f);self->density=self->brightness*self->brightness;}}
void Aud_ModalVoice_SetDamping(Aud_ModalVoice *self,float d){if(self)self->damping=aud_fclamp(d,0.0f,1.0f);}
float Aud_ModalVoice_GetAux(Aud_ModalVoice *self){return(self!=NULL)?self->aux:0.0f;}
float Aud_ModalVoice_Process(Aud_ModalVoice *self,bool trigger){if(self==NULL)return 0.0f;
 float brightness=self->brightness+0.25f*self->accent*(1.0f-self->brightness),damping=self->damping+0.25f*self->accent*(1.0f-self->damping);
 const float range=self->sustain?36.0f:60.0f,f=self->sustain?4.0f*self->f0:2.0f*self->f0;
 const float cutoff=aud_fmin(f*powf(2.0f,(1.0f/12.0f)*((brightness*(2.0f-brightness)-0.5f)*range)),0.499f);
 const float q=self->sustain?0.7f:1.5f;float temp=0.0f;
 if(self->sustain){float df=0.00005f+0.99995f*self->density*self->density;Aud_Dust_SetDensity(&self->dust,df);
  temp=Aud_Dust_Process(&self->dust)*(4.0f-df*3.0f)*self->accent;}
 else if(trigger||self->trig){float att=1.0f-damping*0.5f,amp=(0.12f+0.08f*self->accent)*att;
  temp=amp*powf(2.0f,(1.0f/12.0f)*(cutoff*cutoff*24.0f))/cutoff;self->trig=false;}
 /* simplified excitation 1-mode SVF: lowpass */
 float g=tanf(AUD_PI*cutoff),r=1.0f/q,h=1.0f/(1.0f+r*g+g*g),s1=self->excitation_filter.state_1[0],s2=self->excitation_filter.state_2[0];
 float hp=(temp-r*g*s1-g*g*s2)*h,bp=g*hp+s1;self->excitation_filter.state_1[0]=g*hp+bp;
 float lp=g*bp+s2;self->excitation_filter.state_2[0]=g*bp+lp;temp=lp;self->aux=temp;
 Aud_Resonator_SetBrightness(&self->resonator,brightness);Aud_Resonator_SetDamping(&self->resonator,damping);
 return Aud_Resonator_Process(&self->resonator,temp);}
