#ifndef AUD_KARPLUSSTRING_H
#define AUD_KARPLUSSTRING_H
#include <stdint.h>
#include <stdbool.h>
#include "aud_delayline.h"
#include "aud_dcblock.h"
#include "aud_onepole.h"
#define AUD_STRING_MAX_DELAY 2048
typedef struct {float sample_rate,frequency,brightness,damping,nonlinearity;float del_buf[AUD_STRING_MAX_DELAY];
 Aud_DelayLine delay;Aud_DcBlock dc;Aud_OnePole filter;} Aud_String;
void Aud_String_Init(Aud_String *self,float sr);
void Aud_String_Reset(Aud_String *self);
float Aud_String_Process(Aud_String *self,float in);
void Aud_String_SetFreq(Aud_String *self,float f);
void Aud_String_SetBrightness(Aud_String *self,float b);
void Aud_String_SetDamping(Aud_String *self,float d);
void Aud_String_SetNonLinearity(Aud_String *self,float nl);
#endif
