#ifndef AUD_VARIABLESHAPEOSC_H
#define AUD_VARIABLESHAPEOSC_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float sample_rate; bool enable_sync;
    float master_phase, slave_phase, next_sample, previous_pw; bool high;
    float master_frequency, slave_frequency, pw, waveshape;
} Aud_VariableShapeOscillator;

void  Aud_VariableShapeOscillator_Init(Aud_VariableShapeOscillator *self, float sr);
float Aud_VariableShapeOscillator_Process(Aud_VariableShapeOscillator *self);
void  Aud_VariableShapeOscillator_SetFreq(Aud_VariableShapeOscillator *self, float f);
void  Aud_VariableShapeOscillator_SetPW(Aud_VariableShapeOscillator *self, float pw);
void  Aud_VariableShapeOscillator_SetWaveshape(Aud_VariableShapeOscillator *self, float ws);
void  Aud_VariableShapeOscillator_SetSync(Aud_VariableShapeOscillator *self, bool en);
void  Aud_VariableShapeOscillator_SetSyncFreq(Aud_VariableShapeOscillator *self, float f);
#endif
