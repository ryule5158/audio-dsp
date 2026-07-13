#include "aud_pluck.h"
#include <stdlib.h>
#include <math.h>
#include <stddef.h>
#define PLUKMIN 64
static void pluck_Reinit(Aud_Pluck *self){if(self==NULL||self->buf==NULL)return;
 self->npts=(int32_t)(self->decay*(float)(self->maxpts-PLUKMIN)+PLUKMIN);
 self->sicps=((float)self->npts*256.0f+128.0f)*(1.0f/self->sample_rate);
 float*ap=self->buf;int n;for(n=self->npts;n--;){float val=(float)rand()/(float)RAND_MAX;*ap++=val*2.0f-1.0f;}self->phs256=0;}

void Aud_Pluck_Init(Aud_Pluck *self,float sr,float*buf,int32_t npts,int32_t mode){if(self==NULL)return;
 self->amp=0.5f;self->freq=300.0f;self->decay=1.0f;self->sample_rate=sr;self->mode=mode;
 self->maxpts=npts;self->npts=npts;self->buf=buf;pluck_Reinit(self);
 self->sicps=(npts*256.0f+128.0f)*(1.0f/sr);self->init=1;}

float Aud_Pluck_Process(Aud_Pluck *self,float*trig){if(self==NULL||self->buf==NULL)return 0.0f;
 if(trig&&*trig!=0.0f){self->init=0;pluck_Reinit(self);}if(self->init)return 0.0f;
 float dampmin=0.42f,coeff;switch(self->mode){case AUD_PLUCK_MODE_RECURSIVE:coeff=((0.5f-dampmin)*self->damp)+dampmin;break;
 case AUD_PLUCK_MODE_WEIGHTED_AVERAGE:coeff=0.05f+(self->damp*0.90f);break;default:coeff=0.5f;break;}
 int32_t phsinc=(int32_t)(self->freq*self->sicps),phs256=self->phs256,ltwopi=self->npts<<8,offset=phs256>>8;
 float*fp=self->buf+offset,diff=fp[1]-fp[0],frac=(float)(phs256&255)/256.0f,out=(fp[0]+diff*frac)*self->amp;
 if((phs256+=phsinc)>=ltwopi){phs256-=ltwopi;fp=self->buf;float preval=fp[0];fp[0]=fp[self->npts];fp++;int nn=self->npts;
  do{switch(self->mode){case AUD_PLUCK_MODE_RECURSIVE:preval=(*fp+preval)*coeff;break;
  case AUD_PLUCK_MODE_WEIGHTED_AVERAGE:preval=(*fp*coeff)+(preval*(1.0f-coeff));break;default:break;}
  *fp++=preval;}while(--nn);}self->phs256=phs256;return out;}

void Aud_Pluck_SetAmp(Aud_Pluck *self,float a){if(self)self->amp=a;}
void Aud_Pluck_SetFreq(Aud_Pluck *self,float f){if(self)self->freq=f;}
void Aud_Pluck_SetDecay(Aud_Pluck *self,float d){if(self)self->decay=d;}
void Aud_Pluck_SetDamp(Aud_Pluck *self,float d){if(self)self->damp=d;}
void Aud_Pluck_SetMode(Aud_Pluck *self,int32_t m){if(self)self->mode=m;}
