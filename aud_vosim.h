#ifndef AUD_VOSIM_H
#define AUD_VOSIM_H
#include <stdint.h>

typedef struct {
    float sample_rate;
    float carrier_phase, formant_1_phase, formant_2_phase;
    float carrier_frequency, formant_1_frequency, formant_2_frequency;
    float carrier_shape;
} Aud_VosimOscillator;

void  Aud_VosimOscillator_Init(Aud_VosimOscillator *self, float sr);
float Aud_VosimOscillator_Process(Aud_VosimOscillator *self);
void  Aud_VosimOscillator_SetFreq(Aud_VosimOscillator *self, float f);
void  Aud_VosimOscillator_SetForm1Freq(Aud_VosimOscillator *self, float f);
void  Aud_VosimOscillator_SetForm2Freq(Aud_VosimOscillator *self, float f);
void  Aud_VosimOscillator_SetShape(Aud_VosimOscillator *self, float shape);
#endif
