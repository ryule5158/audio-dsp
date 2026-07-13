#ifndef AUD_DECIMATOR_H
#define AUD_DECIMATOR_H
#include <stdint.h>
#include <stdbool.h>
#define AUD_DECIMATOR_MAX_BITS 16

typedef struct {
    float    downsample_factor, bitcrush_factor;
    uint32_t bits_to_crush, inc, threshold;
    float    downsampled, bitcrushed;
    bool     smooth_crushing;
    float    bit_overflow;
} Aud_Decimator;

void  Aud_Decimator_Init(Aud_Decimator *self);
float Aud_Decimator_Process(Aud_Decimator *self, float input);
void  Aud_Decimator_SetDownsampleFactor(Aud_Decimator *self, float f);
void  Aud_Decimator_SetBitcrushFactor(Aud_Decimator *self, float f);
void  Aud_Decimator_SetBitsToCrush(Aud_Decimator *self, uint8_t bits);
void  Aud_Decimator_SetSmoothCrushing(Aud_Decimator *self, bool en);
bool  Aud_Decimator_GetSmoothCrushing(Aud_Decimator *self);
float Aud_Decimator_GetDownsampleFactor(Aud_Decimator *self);
float Aud_Decimator_GetBitcrushFactor(Aud_Decimator *self);
int   Aud_Decimator_GetBitsToCrush(Aud_Decimator *self);
#endif
