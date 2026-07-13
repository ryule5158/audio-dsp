#include "aud_wavefolder.h"
#include <math.h>
#include <stddef.h>
void Aud_Wavefolder_Init(Aud_Wavefolder *self){if(self==NULL)return;self->gain=1.0f;self->offset=0.0f;}
float Aud_Wavefolder_Process(Aud_Wavefolder *self,float in){
 if(self==NULL)return in;in+=self->offset;in*=self->gain;
 float ft=floorf((in+1.0f)*0.5f);float sgn=((int)ft%2==0)?1.0f:-1.0f;return sgn*(in-2.0f*ft);}
void Aud_Wavefolder_SetGain(Aud_Wavefolder *self,float g){if(self)self->gain=g;}
void Aud_Wavefolder_SetOffset(Aud_Wavefolder *self,float o){if(self)self->offset=o;}
