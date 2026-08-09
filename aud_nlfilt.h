/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef AUD_NLFILT_H
#define AUD_NLFILT_H
#include <stdint.h>
#include <stdlib.h>
#define AUD_NLFILT_MAX_DELAY 1024

typedef struct {
    float   in, a, b, d, C, L;
    float   delay[AUD_NLFILT_MAX_DELAY];
    int32_t point;
} Aud_NlFilt;

void Aud_NlFilt_Init(Aud_NlFilt *self);
void Aud_NlFilt_ProcessBlock(Aud_NlFilt *self, float *in, float *out, uint32_t size);
void Aud_NlFilt_SetCoefficients(Aud_NlFilt *self, float a, float b, float d, float C, float L);
void Aud_NlFilt_SetA(Aud_NlFilt *self, float a);
void Aud_NlFilt_SetB(Aud_NlFilt *self, float b);
void Aud_NlFilt_SetD(Aud_NlFilt *self, float d);
void Aud_NlFilt_SetC(Aud_NlFilt *self, float C);
void Aud_NlFilt_SetL(Aud_NlFilt *self, float L);

#endif
