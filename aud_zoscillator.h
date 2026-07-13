#ifndef AUD_ZOSCILLATOR_H
#define AUD_ZOSCILLATOR_H
#include <stdint.h>

typedef struct {
    float sample_rate;
    float carrier_phase, discontinuity_phase, formant_phase, next_sample;
    float carrier_frequency, formant_frequency;
    float carrier_shape, shape_new, mode, mode_new;
} Aud_ZOscillator;

void  Aud_ZOscillator_Init(Aud_ZOscillator *self, float sr);
float Aud_ZOscillator_Process(Aud_ZOscillator *self);
void  Aud_ZOscillator_SetFreq(Aud_ZOscillator *self, float f);
void  Aud_ZOscillator_SetFormantFreq(Aud_ZOscillator *self, float f);
void  Aud_ZOscillator_SetShape(Aud_ZOscillator *self, float shape);
void  Aud_ZOscillator_SetMode(Aud_ZOscillator *self, float mode);
#endif
