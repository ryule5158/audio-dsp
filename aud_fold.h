#ifndef AUD_FOLD_H
#define AUD_FOLD_H
#include <stdint.h>
typedef struct { float incr, index, value; int sample_index; } Aud_Fold;
void Aud_Fold_Init(Aud_Fold *self);
float Aud_Fold_Process(Aud_Fold *self, float in);
void Aud_Fold_SetIncrement(Aud_Fold *self, float incr);
#endif
