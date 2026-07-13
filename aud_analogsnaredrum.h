#ifndef AUD_ANALOGSNAREDRUM_H
#define AUD_ANALOGSNAREDRUM_H
#include <stdint.h>
#include <stdbool.h>
#include "aud_svf.h"
#define AUD_ANALOGSD_NUM_MODES 5
typedef struct {float sample_rate,f0,accent,decay,tone,snappy;bool sustain,trig;int pulse_remaining_samples;
 float pulse,pulse_height,pulse_lp,noise_envelope,sustain_gain;float phase[AUD_ANALOGSD_NUM_MODES];Aud_Svf resonator[AUD_ANALOGSD_NUM_MODES],noise_filter;} Aud_AnalogSnareDrum;
void Aud_AnalogSnareDrum_Init(Aud_AnalogSnareDrum *self,float sr);
float Aud_AnalogSnareDrum_Process(Aud_AnalogSnareDrum *self,bool trigger);
void Aud_AnalogSnareDrum_Trig(Aud_AnalogSnareDrum *self);
void Aud_AnalogSnareDrum_SetSustain(Aud_AnalogSnareDrum *self,bool s);
void Aud_AnalogSnareDrum_SetAccent(Aud_AnalogSnareDrum *self,float a);
void Aud_AnalogSnareDrum_SetFreq(Aud_AnalogSnareDrum *self,float f);
void Aud_AnalogSnareDrum_SetTone(Aud_AnalogSnareDrum *self,float t);
void Aud_AnalogSnareDrum_SetDecay(Aud_AnalogSnareDrum *self,float d);
void Aud_AnalogSnareDrum_SetSnappy(Aud_AnalogSnareDrum *self,float s);
#endif
