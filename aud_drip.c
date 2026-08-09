#include "aud_drip.h"
#include "aud_dsp.h"
#include <math.h>
#include <stdlib.h>
#include <stddef.h>
#define WUTR_SOUND_DECAY 0.95f
#define WUTR_SYSTEM_DECAY 0.996f
#define WUTR_GAIN 1.0f
#define WUTR_NUM_SOURCES 10.0f
#define WUTR_CENTER_FREQ0 450.0f
#define WUTR_CENTER_FREQ1 600.0f
#define WUTR_CENTER_FREQ2 750.0f
#define WUTR_RESON 0.9985f
#define WUTR_FREQ_SWEEP 1.0001f
#define MAX_SHAKE 2000.0f

static int drip_rand(int max){return rand()%(max+1);}
static float drip_noise(void){return(1.0f*rand()-1073741823.5f)*(1.0f/1073741823.0f);}

void Aud_Drip_Init(Aud_Drip *self,float sr,float dettack){
 if(self==NULL)return;
 self->sample_rate=sr;self->dettack=dettack;
 self->num_tubes=10.0f;self->damp=0.2f;self->shake_max=0.0f;self->freq=450.0f;self->freq1=600.0f;self->freq2=720.0f;self->amp=0.3f;
 self->snd_level=0.0f;float tp=2.0f*AUD_PI/sr;
 self->kloop=sr*dettack;self->outputs00=self->outputs01=self->outputs10=self->outputs11=self->outputs20=self->outputs21=0.0f;
 self->total_energy=0.0f;
 self->center_freqs0=self->res_freq0=WUTR_CENTER_FREQ0;
 self->center_freqs1=self->res_freq1=WUTR_CENTER_FREQ1;
 self->center_freqs2=self->res_freq2=WUTR_CENTER_FREQ2;
 self->num_objects_save=self->num_objects=WUTR_NUM_SOURCES;
 self->sound_decay=WUTR_SOUND_DECAY;self->system_decay=WUTR_SYSTEM_DECAY;
 float t=logf(WUTR_NUM_SOURCES)*WUTR_GAIN/WUTR_NUM_SOURCES;self->gains0=self->gains1=self->gains2=t;
 self->coeffs01=WUTR_RESON*WUTR_RESON;self->coeffs00=-WUTR_RESON*2.0f*cosf(WUTR_CENTER_FREQ0*tp);
 self->coeffs11=WUTR_RESON*WUTR_RESON;self->coeffs10=-WUTR_RESON*2.0f*cosf(WUTR_CENTER_FREQ1*tp);
 self->coeffs21=WUTR_RESON*WUTR_RESON;self->coeffs20=-WUTR_RESON*2.0f*cosf(WUTR_CENTER_FREQ2*tp);
 self->shake_energy=self->amp*MAX_SHAKE*0.1f;self->shake_damp=0.0f;
 if(self->shake_energy>MAX_SHAKE)self->shake_energy=MAX_SHAKE;
 self->shake_max_save=0.0f;self->num_objects=10.0f;self->finalZ0=self->finalZ1=self->finalZ2=0.0f;}

float Aud_Drip_Process(Aud_Drip *self,bool trig){
 if(self==NULL)return 0.0f;
 float tp=2.0f*AUD_PI/self->sample_rate;
 if(trig)Aud_Drip_Init(self,self->sample_rate,self->dettack);
 if(self->num_tubes!=0.0f&&self->num_tubes!=self->num_objects){self->num_objects=self->num_tubes;if(self->num_objects<1.0f)self->num_objects=1.0f;}
 if(self->freq!=0.0f&&self->freq!=self->res_freq0){self->res_freq0=self->freq;self->coeffs00=-WUTR_RESON*2.0f*cosf(self->res_freq0*tp);}
 if(self->damp!=0.0f&&self->damp!=self->shake_damp){self->shake_damp=self->damp;self->system_decay=WUTR_SYSTEM_DECAY+(self->shake_damp*0.002f);}
 if(self->shake_max!=0.0f&&self->shake_max!=self->shake_max_save){self->shake_max_save=self->shake_max;self->shake_energy+=self->shake_max_save*MAX_SHAKE*0.1f;if(self->shake_energy>MAX_SHAKE)self->shake_energy=MAX_SHAKE;}
 if(self->freq1!=0.0f&&self->freq1!=self->res_freq1){self->res_freq1=self->freq1;self->coeffs10=-WUTR_RESON*2.0f*cosf(self->res_freq1*tp);}
 if(self->freq2!=0.0f&&self->freq2!=self->res_freq2){self->res_freq2=self->freq2;self->coeffs20=-WUTR_RESON*2.0f*cosf(self->res_freq2*tp);}
 if((--self->kloop)==0.0f)self->shake_energy=0.0f;
 float se=self->shake_energy,sd=self->system_decay,sl=self->snd_level,no=self->num_objects,sc=self->sound_decay;
 se*=sd;sl=se;
 if(drip_rand(32767)<no){int j=drip_rand(3);
  if(j==0){self->center_freqs0=self->res_freq1*(0.75f+0.25f*drip_noise());self->gains0=fabsf(drip_noise());}
  else if(j==1){self->center_freqs1=self->res_freq1*(1.0f+0.25f*drip_noise());self->gains1=fabsf(drip_noise());}
  else{self->center_freqs2=self->res_freq1*(1.25f+0.25f*drip_noise());self->gains2=fabsf(drip_noise());}}
 self->gains0*=WUTR_RESON;if(self->gains0>0.001f){self->center_freqs0*=WUTR_FREQ_SWEEP;self->coeffs00=-WUTR_RESON*2.0f*cosf(self->center_freqs0*tp);}
 self->gains1*=WUTR_RESON;if(self->gains1>0.0f){self->center_freqs1*=WUTR_FREQ_SWEEP;self->coeffs10=-WUTR_RESON*2.0f*cosf(self->center_freqs1*tp);}
 self->gains2*=WUTR_RESON;if(self->gains2>0.001f){self->center_freqs2*=WUTR_FREQ_SWEEP;self->coeffs20=-WUTR_RESON*2.0f*cosf(self->center_freqs2*tp);}
 sl*=sc;float i0=sl*drip_noise(),i1=i0*self->gains1,i2=i0*self->gains2;i0*=self->gains0;
 i0-=self->outputs00*self->coeffs00;i0-=self->outputs01*self->coeffs01;self->outputs01=self->outputs00;self->outputs00=i0;float data=self->gains0*self->outputs00;
 i1-=self->outputs10*self->coeffs10;i1-=self->outputs11*self->coeffs11;self->outputs11=self->outputs10;self->outputs10=i1;data+=self->gains1*self->outputs10;
 i2-=self->outputs20*self->coeffs20;i2-=self->outputs21*self->coeffs21;self->outputs21=self->outputs20;self->outputs20=i2;data+=self->gains2*self->outputs20;
 self->finalZ2=self->finalZ1;self->finalZ1=self->finalZ0;self->finalZ0=data*4.0f;
 float lo=self->finalZ2-self->finalZ0;lo*=0.005f;self->shake_energy=se;self->snd_level=sl;return lo;}
