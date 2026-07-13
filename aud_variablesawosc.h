#ifndef AUD_VARIABLESAWOSC_H
#define AUD_VARIABLESAWOSC_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float sample_rate, phase, next_sample, previous_pw;
    bool  high;
    float frequency, pw, waveshape;
} Aud_VariableSawOscillator;

void  Aud_VariableSawOscillator_Init(Aud_VariableSawOscillator *self, float sr);
float Aud_VariableSawOscillator_Process(Aud_VariableSawOscillator *self);
void  Aud_VariableSawOscillator_SetFreq(Aud_VariableSawOscillator *self, float f);
void  Aud_VariableSawOscillator_SetPW(Aud_VariableSawOscillator *self, float pw);
void  Aud_VariableSawOscillator_SetWaveshape(Aud_VariableSawOscillator *self, float ws);
#endif
