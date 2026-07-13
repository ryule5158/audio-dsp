#ifndef AUD_SAMPLERATEREDUCER_H
#define AUD_SAMPLERATEREDUCER_H
#include <stdint.h>
typedef struct { float frequency, phase, sample, previous_sample, next_sample; } Aud_SampleRateReducer;
void Aud_SampleRateReducer_Init(Aud_SampleRateReducer *self);
float Aud_SampleRateReducer_Process(Aud_SampleRateReducer *self, float in);
void Aud_SampleRateReducer_SetFreq(Aud_SampleRateReducer *self, float f);
#endif
