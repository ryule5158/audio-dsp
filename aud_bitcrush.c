#include "aud_bitcrush.h"
#include <math.h>
#include <stddef.h>
void Aud_Bitcrush_Init(Aud_Bitcrush *self,float sr){if(self==NULL)return;self->bit_depth=8;self->crush_rate=10000.0f;self->sample_rate=sr;Aud_Fold_Init(&self->fold);}
float Aud_Bitcrush_Process(Aud_Bitcrush *self,float in){if(self==NULL)return in;
 float bits=powf(2.0f,(float)self->bit_depth);float foldamt=self->sample_rate/self->crush_rate;
 float out=in*65536.0f;out+=32768.0f;out*=(bits/65536.0f);out=floorf(out);out=out*(65536.0f/bits)-32768.0f;
 Aud_Fold_SetIncrement(&self->fold,foldamt);out=Aud_Fold_Process(&self->fold,out);return out/65536.0f;}
void Aud_Bitcrush_SetBitDepth(Aud_Bitcrush *self,int b){if(self)self->bit_depth=b;}
void Aud_Bitcrush_SetCrushRate(Aud_Bitcrush *self,float r){if(self)self->crush_rate=r;}
