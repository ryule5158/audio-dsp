/**
 * @file    aud_particle.h
 * @brief   Random impulse train through a resonant SVF filter.
 *          Ported from DaisySP Noise/particle (MIT). Original by Emilie Gillet, 2016.
 */
#ifndef AUD_PARTICLE_H
#define AUD_PARTICLE_H
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "aud_svf.h"

typedef struct {
    float sample_rate;
    float aux, frequency, density, gain, spread, resonance;
    bool  sync;
    float rand_phase;
    float rand_freq;
    float pre_gain;
    Aud_Svf filter;
} Aud_Particle;

void  Aud_Particle_Init(Aud_Particle *self, float sample_rate);
float Aud_Particle_Process(Aud_Particle *self);
float Aud_Particle_GetNoise(Aud_Particle *self);
void  Aud_Particle_SetFreq(Aud_Particle *self, float frequency);
void  Aud_Particle_SetResonance(Aud_Particle *self, float resonance);
void  Aud_Particle_SetRandomFreq(Aud_Particle *self, float freq);
void  Aud_Particle_SetDensity(Aud_Particle *self, float density);
void  Aud_Particle_SetGain(Aud_Particle *self, float gain);
void  Aud_Particle_SetSpread(Aud_Particle *self, float spread);
void  Aud_Particle_SetSync(Aud_Particle *self, bool sync);

#endif /* AUD_PARTICLE_H */
