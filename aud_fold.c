#include "aud_fold.h"
#include <stddef.h>
void Aud_Fold_Init(Aud_Fold *self){if(self==NULL)return;self->incr=1000.0f;self->sample_index=0;self->index=0.0f;self->value=0.0f;}
float Aud_Fold_Process(Aud_Fold *self,float in){if(self==NULL)return in;
 if(self->index<(float)self->sample_index){self->index+=self->incr;self->value=in;return in;}self->sample_index++;return self->value;}
void Aud_Fold_SetIncrement(Aud_Fold *self,float incr){if(self)self->incr=incr;}
