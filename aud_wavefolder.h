#ifndef AUD_WAVEFOLDER_H
#define AUD_WAVEFOLDER_H
#include <stdint.h>
typedef struct { float gain, offset; } Aud_Wavefolder;
void Aud_Wavefolder_Init(Aud_Wavefolder *self);
float Aud_Wavefolder_Process(Aud_Wavefolder *self, float in);
void Aud_Wavefolder_SetGain(Aud_Wavefolder *self, float gain);
void Aud_Wavefolder_SetOffset(Aud_Wavefolder *self, float offset);
#endif
