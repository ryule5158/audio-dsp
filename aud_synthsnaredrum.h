#ifndef AUD_SYNTHSNAREDRUM_H
#define AUD_SYNTHSNAREDRUM_H
#include <stdint.h>
#include <stdbool.h>
#include "aud_svf.h"
typedef struct {float sample_rate,f0,accent,fm_amount,fm_amount_sq,decay,snappy;bool sustain,trig;
 float phase[2],drum_amplitude,snare_amplitude,fm,sustain_gain;int hold_counter;Aud_Svf drum_lp,snare_hp,snare_lp;} Aud_SyntheticSnareDrum;
void Aud_SyntheticSnareDrum_Init(Aud_SyntheticSnareDrum *self,float sr);
float Aud_SyntheticSnareDrum_Process(Aud_SyntheticSnareDrum *self,bool trigger);
void Aud_SyntheticSnareDrum_Trig(Aud_SyntheticSnareDrum *self);
void Aud_SyntheticSnareDrum_SetSustain(Aud_SyntheticSnareDrum *self,bool s);
void Aud_SyntheticSnareDrum_SetAccent(Aud_SyntheticSnareDrum *self,float a);
void Aud_SyntheticSnareDrum_SetFreq(Aud_SyntheticSnareDrum *self,float f);
void Aud_SyntheticSnareDrum_SetFmAmount(Aud_SyntheticSnareDrum *self,float a);
void Aud_SyntheticSnareDrum_SetDecay(Aud_SyntheticSnareDrum *self,float d);
void Aud_SyntheticSnareDrum_SetSnappy(Aud_SyntheticSnareDrum *self,float s);
#endif
