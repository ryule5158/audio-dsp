#ifndef AUD_FORMANTOSC_H
#define AUD_FORMANTOSC_H
#include <stdint.h>

typedef struct {
    float carrier_phase, formant_phase, next_sample;
    float carrier_frequency, formant_frequency, phase_shift, ps_inc;
    float sample_rate;
} Aud_FormantOscillator;

void  Aud_FormantOscillator_Init(Aud_FormantOscillator *self, float sample_rate);
float Aud_FormantOscillator_Process(Aud_FormantOscillator *self);
void  Aud_FormantOscillator_SetFormantFreq(Aud_FormantOscillator *self, float freq);
void  Aud_FormantOscillator_SetCarrierFreq(Aud_FormantOscillator *self, float freq);
void  Aud_FormantOscillator_SetPhaseShift(Aud_FormantOscillator *self, float ps);
#endif
