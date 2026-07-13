#include "aud_decimator.h"
#include <stddef.h>

void Aud_Decimator_Init(Aud_Decimator *self)
{
    if (self == NULL) return;
    self->downsample_factor = 1.0f;
    self->bitcrush_factor   = 0.0f;
    self->downsampled = self->bitcrushed = 0.0f;
    self->inc = self->threshold = 0;
    self->smooth_crushing = false;
    self->bit_overflow = 1.0f;
}

float Aud_Decimator_Process(Aud_Decimator *self, float input)
{
    if (self == NULL) return input;
    int32_t temp;
    self->threshold = (uint32_t)((self->downsample_factor * self->downsample_factor) * 96.0f);
    self->inc += 1;
    if (self->inc > self->threshold) { self->inc = 0; self->downsampled = input; }

    if (self->smooth_crushing) {
        temp = (int32_t)(self->downsampled * 65536.0f * self->bit_overflow);
        temp >>= self->bits_to_crush + 1;
        temp <<= self->bits_to_crush + 1;
        self->bitcrushed = (float)temp / (65536.0f * self->bit_overflow);
    } else {
        temp = (int32_t)(self->downsampled * 65536.0f);
        temp >>= self->bits_to_crush;
        temp <<= self->bits_to_crush;
        self->bitcrushed = (float)temp / 65536.0f;
    }
    return self->bitcrushed;
}

void Aud_Decimator_SetDownsampleFactor(Aud_Decimator *self, float f)
    { if (self) self->downsample_factor = f; }
void Aud_Decimator_SetBitcrushFactor(Aud_Decimator *self, float f)
    { if (self) { self->bitcrush_factor = f;
      self->bits_to_crush = (uint32_t)(f * AUD_DECIMATOR_MAX_BITS);
      self->bit_overflow = 2.0f - (f * 16.0f) + (float)self->bits_to_crush; } }
void Aud_Decimator_SetBitsToCrush(Aud_Decimator *self, uint8_t bits)
    { if (self) { self->bits_to_crush = (bits <= AUD_DECIMATOR_MAX_BITS) ? bits : AUD_DECIMATOR_MAX_BITS;
      self->smooth_crushing = false; } }
void Aud_Decimator_SetSmoothCrushing(Aud_Decimator *self, bool en)
    { if (self) self->smooth_crushing = en; }
bool Aud_Decimator_GetSmoothCrushing(Aud_Decimator *self)
    { return (self != NULL) ? self->smooth_crushing : false; }
float Aud_Decimator_GetDownsampleFactor(Aud_Decimator *self)
    { return (self != NULL) ? self->downsample_factor : 0.0f; }
float Aud_Decimator_GetBitcrushFactor(Aud_Decimator *self)
    { return (self != NULL) ? self->bitcrush_factor : 0.0f; }
int Aud_Decimator_GetBitsToCrush(Aud_Decimator *self)
    { return (self != NULL) ? (int)self->bits_to_crush : 0; }
