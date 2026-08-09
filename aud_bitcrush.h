/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef AUD_BITCRUSH_H
#define AUD_BITCRUSH_H
#include <stdint.h>
#include "aud_fold.h"
typedef struct { float sample_rate, crush_rate; int bit_depth; Aud_Fold fold; } Aud_Bitcrush;
void Aud_Bitcrush_Init(Aud_Bitcrush *self, float sr);
float Aud_Bitcrush_Process(Aud_Bitcrush *self, float in);
void Aud_Bitcrush_SetBitDepth(Aud_Bitcrush *self, int bits);
void Aud_Bitcrush_SetCrushRate(Aud_Bitcrush *self, float rate);
#endif
