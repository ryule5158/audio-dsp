#ifndef AUD_SYNTHBASSDRUM_H
#define AUD_SYNTHBASSDRUM_H
#include <stdint.h>
#include <stdbool.h>
#include "aud_svf.h"
typedef struct {float lp,hp;Aud_Svf filter;} Aud_SyntheticBassDrumClick;
typedef struct {float lp,hp;} Aud_SyntheticBassDrumAttackNoise;
typedef struct {float sample_rate,f0,new_f0,tone,decay,accent,dirtiness,fm_envelope_amount,fm_envelope_decay,sustain_gain;
 bool sustain,trig;float phase,phase_noise,fm,fm_lp,body_env,body_env_lp,transient_env,transient_env_lp,tone_lp;
 int body_env_pulse_width,fm_pulse_width;Aud_SyntheticBassDrumClick click;Aud_SyntheticBassDrumAttackNoise noise;} Aud_SyntheticBassDrum;
void Aud_SyntheticBassDrumClick_Init(Aud_SyntheticBassDrumClick *self,float sr);
float Aud_SyntheticBassDrumClick_Process(Aud_SyntheticBassDrumClick *self,float in);
void Aud_SyntheticBassDrumAttackNoise_Init(Aud_SyntheticBassDrumAttackNoise *self);
float Aud_SyntheticBassDrumAttackNoise_Process(Aud_SyntheticBassDrumAttackNoise *self);
void Aud_SyntheticBassDrum_Init(Aud_SyntheticBassDrum *self,float sr);
float Aud_SyntheticBassDrum_Process(Aud_SyntheticBassDrum *self,bool trigger);
void Aud_SyntheticBassDrum_Trig(Aud_SyntheticBassDrum *self);
void Aud_SyntheticBassDrum_SetSustain(Aud_SyntheticBassDrum *self,bool s);
void Aud_SyntheticBassDrum_SetAccent(Aud_SyntheticBassDrum *self,float a);
void Aud_SyntheticBassDrum_SetFreq(Aud_SyntheticBassDrum *self,float f);
void Aud_SyntheticBassDrum_SetTone(Aud_SyntheticBassDrum *self,float t);
void Aud_SyntheticBassDrum_SetDecay(Aud_SyntheticBassDrum *self,float d);
void Aud_SyntheticBassDrum_SetDirtiness(Aud_SyntheticBassDrum *self,float d);
void Aud_SyntheticBassDrum_SetFmEnvelopeAmount(Aud_SyntheticBassDrum *self,float a);
void Aud_SyntheticBassDrum_SetFmEnvelopeDecay(Aud_SyntheticBassDrum *self,float d);
#endif
