#ifndef AUD_OSCILLATORBANK_H
#define AUD_OSCILLATORBANK_H
#include <stdint.h>
#include <stdbool.h>
typedef struct {float phase,next_sample,frequency,saw_8_gain,saw_4_gain,saw_2_gain,saw_1_gain,gain,sample_rate;
 float registration[7],unshifted_registration[7];int segment;bool recalc,recalc_gain;} Aud_OscillatorBank;
void Aud_OscillatorBank_Init(Aud_OscillatorBank *self,float sr);
float Aud_OscillatorBank_Process(Aud_OscillatorBank *self);
void Aud_OscillatorBank_SetFreq(Aud_OscillatorBank *self,float f);
void Aud_OscillatorBank_SetAmplitudes(Aud_OscillatorBank *self,const float*amps);
void Aud_OscillatorBank_SetSingleAmp(Aud_OscillatorBank *self,float amp,int idx);
void Aud_OscillatorBank_SetGain(Aud_OscillatorBank *self,float gain);
#endif
