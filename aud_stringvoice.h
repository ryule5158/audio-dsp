#ifndef AUD_STRINGVOICE_H
#define AUD_STRINGVOICE_H
#include <stdint.h>
#include <stdbool.h>
#include "aud_svf.h"
#include "aud_karplusstring.h"
#include "aud_dust.h"

typedef struct {
    float sample_rate, f0, brightness, damping, density, accent, aux;
    bool sustain, trig;
    Aud_Dust dust;
    Aud_Svf  excitation_filter;
    Aud_String string;
    uint32_t remaining_noise_samples;
} Aud_StringVoice;

void  Aud_StringVoice_Init(Aud_StringVoice *self, float sr);
void  Aud_StringVoice_Reset(Aud_StringVoice *self);
float Aud_StringVoice_Process(Aud_StringVoice *self, bool trigger);
void  Aud_StringVoice_SetSustain(Aud_StringVoice *self, bool s);
void  Aud_StringVoice_Trig(Aud_StringVoice *self);
void  Aud_StringVoice_SetFreq(Aud_StringVoice *self, float f);
void  Aud_StringVoice_SetAccent(Aud_StringVoice *self, float a);
void  Aud_StringVoice_SetStructure(Aud_StringVoice *self, float s);
void  Aud_StringVoice_SetBrightness(Aud_StringVoice *self, float b);
void  Aud_StringVoice_SetDamping(Aud_StringVoice *self, float d);
float Aud_StringVoice_GetAux(Aud_StringVoice *self);
#endif
