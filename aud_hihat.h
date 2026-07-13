#ifndef AUD_HIHAT_H
#define AUD_HIHAT_H
#include <stdint.h>
#include <stdbool.h>
#include "aud_svf.h"
#include "aud_osc.h"
typedef struct {uint32_t phase[6];} Aud_SquareNoise;
typedef struct {Aud_Osc oscillator[6];float sample_rate;} Aud_RingModNoise;
typedef struct {float sample_rate,accent,f0,tone,decay,noisiness,sustain_gain,envelope,noise_clock,noise_sample;
 bool sustain,trig;Aud_SquareNoise metallic_noise;Aud_Svf noise_coloration_svf,hpf;} Aud_HiHat;
void Aud_SquareNoise_Init(Aud_SquareNoise *self,float sr);
float Aud_SquareNoise_Process(Aud_SquareNoise *self,float f0);
void Aud_RingModNoise_Init(Aud_RingModNoise *self,float sr);
float Aud_RingModNoise_Process(Aud_RingModNoise *self,float f0);
void Aud_HiHat_Init(Aud_HiHat *self,float sr);
float Aud_HiHat_Process(Aud_HiHat *self,bool trigger);
void Aud_HiHat_Trig(Aud_HiHat *self);
void Aud_HiHat_SetSustain(Aud_HiHat *self,bool s);
void Aud_HiHat_SetAccent(Aud_HiHat *self,float a);
void Aud_HiHat_SetFreq(Aud_HiHat *self,float f);
void Aud_HiHat_SetTone(Aud_HiHat *self,float t);
void Aud_HiHat_SetDecay(Aud_HiHat *self,float d);
void Aud_HiHat_SetNoisiness(Aud_HiHat *self,float n);
#endif
