#ifndef AUD_ANALOGBASSDRUM_H
#define AUD_ANALOGBASSDRUM_H
#include <stdint.h>
#include <stdbool.h>
#include "aud_svf.h"
typedef struct {float sample_rate,f0,tone,decay,accent,attack_fm_amount,self_fm_amount,sustain_gain;bool sustain,trig;
 int pulse_remaining_samples,fm_pulse_remaining_samples;float pulse,pulse_height,pulse_lp,fm_pulse_lp,retrig_pulse,lp_out,tone_lp,phase;Aud_Svf resonator;} Aud_AnalogBassDrum;
void Aud_AnalogBassDrum_Init(Aud_AnalogBassDrum *self,float sr);
float Aud_AnalogBassDrum_Process(Aud_AnalogBassDrum *self,bool trigger);
void Aud_AnalogBassDrum_Trig(Aud_AnalogBassDrum *self);
void Aud_AnalogBassDrum_SetSustain(Aud_AnalogBassDrum *self,bool s);
void Aud_AnalogBassDrum_SetAccent(Aud_AnalogBassDrum *self,float a);
void Aud_AnalogBassDrum_SetFreq(Aud_AnalogBassDrum *self,float f);
void Aud_AnalogBassDrum_SetTone(Aud_AnalogBassDrum *self,float t);
void Aud_AnalogBassDrum_SetDecay(Aud_AnalogBassDrum *self,float d);
void Aud_AnalogBassDrum_SetAttackFmAmount(Aud_AnalogBassDrum *self,float a);
void Aud_AnalogBassDrum_SetSelfFmAmount(Aud_AnalogBassDrum *self,float a);
#endif
