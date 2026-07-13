#include "aud_looper.h"
#include <math.h>
#include <string.h>
#include <stddef.h>

static float looper_WindowVal(float in){return sinf(AUD_HALFPI*in);}
static float looper_GetInc(Aud_Looper *self){float inc=1.0f;if(self->half_speed)inc=0.5f;return self->reverse?-inc:inc;}
static float looper_Read(Aud_Looper *self,uint32_t pos){return self->buff[pos];}
static void looper_Write(Aud_Looper *self,uint32_t pos,float val){self->buff[pos]=val;}

void Aud_Looper_Init(Aud_Looper *self,float*mem,uint32_t size){if(self==NULL)return;self->buffer_size=size;self->buff=mem;
 memset(self->buff,0,size*sizeof(float));self->state=AUD_LOOP_STATE_EMPTY;self->mode=AUD_LOOP_NORMAL;self->half_speed=false;
 self->reverse=false;self->rec_queue=false;self->win_idx=0;}

float Aud_Looper_Process(Aud_Looper *self,float input){if(self==NULL)return 0.0f;float sig=0.0f;float inc;bool hitloop=false;
 inc=(self->state==AUD_LOOP_STATE_EMPTY||self->state==AUD_LOOP_STATE_REC_FIRST)?1.0f:looper_GetInc(self);
 self->win_=looper_WindowVal((float)self->win_idx*AUD_LOOPER_WINDOW_FACTOR);
 switch(self->state){
 case AUD_LOOP_STATE_EMPTY:sig=0.0f;break;
 case AUD_LOOP_STATE_REC_FIRST:sig=0.0f;looper_Write(self,(uint32_t)self->pos,input*self->win_);
  if(self->win_idx<AUD_LOOPER_WINDOW_SAMPS-1)self->win_idx++;self->recsize=(uint32_t)self->pos;self->pos+=inc;
  if(self->pos>(float)(self->buffer_size-1)){self->state=AUD_LOOP_STATE_PLAYING;self->recsize=(uint32_t)(self->pos-1.0f);self->pos=0.0f;}break;
 case AUD_LOOP_STATE_PLAYING:sig=looper_Read(self,(uint32_t)self->pos);
  if(self->win_idx<AUD_LOOPER_WINDOW_SAMPS-1){looper_Write(self,(uint32_t)self->pos,sig+input*(1.0f-self->win_));self->win_idx++;}
  self->pos+=inc;if(self->pos>(float)(self->recsize-1)){self->pos=0.0f;hitloop=true;}else if(self->pos<0.0f){self->pos=(float)(self->recsize-1);hitloop=true;}
  if(hitloop&&self->rec_queue&&self->mode==AUD_LOOP_ONETIME_DUB){self->rec_queue=false;self->state=AUD_LOOP_STATE_REC_DUB;self->win_idx=0;}break;
 case AUD_LOOP_STATE_REC_DUB:sig=looper_Read(self,(uint32_t)self->pos);
  switch(self->mode){case AUD_LOOP_REPLACE:looper_Write(self,(uint32_t)self->pos,input*self->win_);break;
   case AUD_LOOP_FRIPPERTRONICS:looper_Write(self,(uint32_t)self->pos,(input*self->win_)+(sig*AUD_LOOPER_FRIP_DECAY));break;
   default:looper_Write(self,(uint32_t)self->pos,(input*self->win_)+sig);break;}
  if(self->win_idx<AUD_LOOPER_WINDOW_SAMPS-1)self->win_idx++;self->pos+=inc;
  if(self->pos>(float)(self->recsize-1)){self->pos=0.0f;hitloop=true;}else if(self->pos<0.0f){self->pos=(float)(self->recsize-1);hitloop=true;}
  if(hitloop&&self->mode==AUD_LOOP_ONETIME_DUB){self->state=AUD_LOOP_STATE_PLAYING;self->win_idx=0;}break;default:break;}
 self->near_beginning=(self->state!=AUD_LOOP_STATE_EMPTY&&!Aud_Looper_Recording(self)&&self->pos<4800.0f);return sig;}

void Aud_Looper_Clear(Aud_Looper *self){if(self)self->state=AUD_LOOP_STATE_EMPTY;}
void Aud_Looper_TrigRecord(Aud_Looper *self){if(self==NULL)return;
 switch(self->state){case AUD_LOOP_STATE_EMPTY:self->pos=0.0f;self->recsize=0;self->state=AUD_LOOP_STATE_REC_FIRST;self->half_speed=false;self->reverse=false;break;
  case AUD_LOOP_STATE_REC_FIRST:case AUD_LOOP_STATE_REC_DUB:self->state=AUD_LOOP_STATE_PLAYING;break;
  case AUD_LOOP_STATE_PLAYING:if(self->mode==AUD_LOOP_ONETIME_DUB)self->rec_queue=true;else self->state=AUD_LOOP_STATE_REC_DUB;break;
  default:self->state=AUD_LOOP_STATE_EMPTY;break;}if(!self->rec_queue)self->win_idx=0;}
bool Aud_Looper_Recording(Aud_Looper *self){return self!=NULL&&(self->state==AUD_LOOP_STATE_REC_DUB||self->state==AUD_LOOP_STATE_REC_FIRST);}
void Aud_Looper_IncrementMode(Aud_Looper *self){if(self){int m=(int)self->mode+1;if(m>3)m=0;self->mode=(Aud_Looper_Mode)m;}}
void Aud_Looper_SetMode(Aud_Looper *self,Aud_Looper_Mode m){if(self)self->mode=m;}
Aud_Looper_Mode Aud_Looper_GetMode(Aud_Looper *self){return(self!=NULL)?self->mode:AUD_LOOP_NORMAL;}
void Aud_Looper_ToggleReverse(Aud_Looper *self){if(self)self->reverse=!self->reverse;}
void Aud_Looper_SetReverse(Aud_Looper *self,bool s){if(self)self->reverse=s;}
bool Aud_Looper_GetReverse(Aud_Looper *self){return(self!=NULL)&&self->reverse;}
void Aud_Looper_ToggleHalfSpeed(Aud_Looper *self){if(self)self->half_speed=!self->half_speed;}
void Aud_Looper_SetHalfSpeed(Aud_Looper *self,bool s){if(self)self->half_speed=s;}
bool Aud_Looper_GetHalfSpeed(Aud_Looper *self){return(self!=NULL)&&self->half_speed;}
bool Aud_Looper_IsNearBeginning(Aud_Looper *self){return(self!=NULL)&&self->near_beginning;}
