#include "aud_reverbsc.h"
#include <math.h>
#include <stdlib.h>
#include <stddef.h>

#define REVSC_OK 0
#define REVSC_NOT_OK 1
#define DEFAULT_SRATE 48000.0
#define DELAYPOS_SHIFT 28
#define DELAYPOS_SCALE 0x10000000
#define DELAYPOS_MASK 0x0FFFFFFF
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static const float kReverbParams[8][4] = {
    {(2473.0f/DEFAULT_SRATE),0.0010f,3.100f,1966.0f},{(2767.0f/DEFAULT_SRATE),0.0011f,3.500f,29491.0f},
    {(3217.0f/DEFAULT_SRATE),0.0017f,1.110f,22937.0f},{(3557.0f/DEFAULT_SRATE),0.0006f,3.973f,9830.0f},
    {(3907.0f/DEFAULT_SRATE),0.0010f,2.341f,20643.0f},{(4127.0f/DEFAULT_SRATE),0.0011f,1.897f,22937.0f},
    {(2143.0f/DEFAULT_SRATE),0.0017f,0.891f,29491.0f},{(1933.0f/DEFAULT_SRATE),0.0006f,3.221f,14417.0f}};
static const float kOutputGain=0.35f,kJpScale=0.25f;

static int DelayLineMaxSamples(float sr,float ipm,int n){float md=kReverbParams[n][0]+(kReverbParams[n][1]*ipm*1.125f);return(int)(md*sr+16.5f);}
static int DelayLineBytesAlloc(float sr,float ipm,int n){return DelayLineMaxSamples(sr,ipm,n)*(int)sizeof(float);}

static void NextRandomLineseg(Aud_ReverbSc *self,Aud_ReverbScDl *lp,int n){
 if(lp->seed_val<0)lp->seed_val+=0x10000;lp->seed_val=(lp->seed_val*15625+1)&0xFFFF;
 if(lp->seed_val>=0x8000)lp->seed_val-=0x10000;
 lp->rand_line_cnt=(int)((self->sample_rate/kReverbParams[n][2])+0.5f);
 float prv_del=(float)lp->write_pos-((float)lp->read_pos+((float)lp->read_pos_frac/(float)DELAYPOS_SCALE));
 while(prv_del<0.0f)prv_del+=lp->buffer_size;prv_del/=self->sample_rate;
 float nxt_del=(float)lp->seed_val*kReverbParams[n][1]/32768.0f;
 nxt_del=kReverbParams[n][0]+(nxt_del*(float)self->i_pitch_mod);
 float phi=(prv_del-nxt_del)/(float)lp->rand_line_cnt;phi=phi*self->sample_rate+1.0f;
 lp->read_pos_frac_inc=(int)(phi*DELAYPOS_SCALE+0.5f);}

static int InitDelayLine(Aud_ReverbSc *self,Aud_ReverbScDl *lp,int n){
 lp->buffer_size=DelayLineMaxSamples(self->sample_rate,1,n);lp->dummy=0;lp->write_pos=0;
 lp->seed_val=(int)(kReverbParams[n][3]+0.5f);
 float rp=(float)lp->seed_val*kReverbParams[n][1]/32768.0f;
 rp=kReverbParams[n][0]+(rp*(float)self->i_pitch_mod);rp=(float)lp->buffer_size-(rp*self->sample_rate);
 lp->read_pos=(int)rp;rp=(rp-(float)lp->read_pos)*(float)DELAYPOS_SCALE;lp->read_pos_frac=(int)(rp+0.5f);
 NextRandomLineseg(self,lp,n);lp->filter_state=0.0f;int i;for(i=0;i<lp->buffer_size;i++)lp->buf[i]=0.0f;return REVSC_OK;}

int Aud_ReverbSc_Init(Aud_ReverbSc *self,float sr){
 if(self==NULL)return REVSC_NOT_OK;self->i_sample_rate=sr;self->sample_rate=sr;self->feedback=0.97f;
 self->lpfreq=10000.0f;self->i_pitch_mod=1.0f;self->i_skip_init=0.0f;self->damp_fact=1.0f;
 self->prv_lpfreq=0.0f;self->init_done=1;int i,n_bytes=0;
 for(i=0;i<8;i++){if(n_bytes>AUD_REVERBSC_MAX_SIZE)return REVSC_NOT_OK;
  self->delay_lines[i].buf=self->aux+n_bytes;InitDelayLine(self,&self->delay_lines[i],i);
  n_bytes+=DelayLineBytesAlloc(sr,1,i);}return REVSC_OK;}

int Aud_ReverbSc_Process(Aud_ReverbSc *self,float in1,float in2,float*out1,float*out2){
 if(self==NULL||out1==NULL||out2==NULL)return REVSC_NOT_OK;
 if(self->init_done<=0)return REVSC_NOT_OK;float damp_fact=self->damp_fact;
 if(self->lpfreq!=self->prv_lpfreq){self->prv_lpfreq=self->lpfreq;
  damp_fact=2.0f-cosf(self->prv_lpfreq*(2.0f*(float)M_PI)/self->sample_rate);
  damp_fact=self->damp_fact=damp_fact-sqrtf(damp_fact*damp_fact-1.0f);}
 float a_in_l=0.0f,a_out_l=0.0f,a_out_r=0.0f;uint32_t n;
 for(n=0;n<8;n++)a_in_l+=self->delay_lines[n].filter_state;a_in_l*=kJpScale;
 float a_in_r=a_in_l+in2;a_in_l+=in1;
 for(n=0;n<8;n++){Aud_ReverbScDl*lp=&self->delay_lines[n];int bs=lp->buffer_size;
  lp->buf[lp->write_pos]=(float)((n&1?a_in_r:a_in_l)-lp->filter_state);
  if(++lp->write_pos>=bs)lp->write_pos-=bs;
  if(lp->read_pos_frac>=DELAYPOS_SCALE){lp->read_pos+=(lp->read_pos_frac>>DELAYPOS_SHIFT);lp->read_pos_frac&=DELAYPOS_MASK;}
  if(lp->read_pos>=bs)lp->read_pos-=bs;int rp=lp->read_pos;float frac=(float)lp->read_pos_frac*(1.0f/(float)DELAYPOS_SCALE);
  float a2=frac*frac;a2-=1.0f;a2*=(1.0f/6.0f);float a1=frac;a1+=1.0f;a1*=0.5f;
  float am1=a1-1.0f,a0=3.0f*a2;a1-=a0;am1-=a2;a0-=frac;
  float vm1,v0,v1,v2;
  if(rp>0&&rp<(bs-2)){vm1=lp->buf[rp-1];v0=lp->buf[rp];v1=lp->buf[rp+1];v2=lp->buf[rp+2];}
  else{if(--rp<0)rp+=bs;vm1=lp->buf[rp];if(++rp>=bs)rp-=bs;v0=lp->buf[rp];
   if(++rp>=bs)rp-=bs;v1=lp->buf[rp];if(++rp>=bs)rp-=bs;v2=lp->buf[rp];}
  v0=(am1*vm1+a0*v0+a1*v1+a2*v2)*frac+v0;lp->read_pos_frac+=lp->read_pos_frac_inc;
  v0*=(float)self->feedback;v0=(lp->filter_state-v0)*damp_fact+v0;lp->filter_state=v0;
  if(n&1)a_out_r+=v0;else a_out_l+=v0;if(--(lp->rand_line_cnt)<=0)NextRandomLineseg(self,lp,n);}
 *out1=a_out_l*kOutputGain;*out2=a_out_r*kOutputGain;return REVSC_OK;}
void Aud_ReverbSc_SetFeedback(Aud_ReverbSc *self,float fb){if(self)self->feedback=fb;}
void Aud_ReverbSc_SetLpFreq(Aud_ReverbSc *self,float f){if(self)self->lpfreq=f;}
