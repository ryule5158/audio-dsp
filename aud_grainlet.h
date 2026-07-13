/**
 * @file    aud_grainlet.h
 * @brief   Granular oscillator — phase-distorted sine × carrier.
 *          Ported from DaisySP Noise/grainlet (MIT). Original by Emilie Gillet, 2016.
 */
#ifndef AUD_GRAINLET_H
#define AUD_GRAINLET_H
#include <stdint.h>

typedef struct {
    float carrier_phase;
    float formant_phase;
    float next_sample;
    float carrier_frequency;
    float formant_frequency;
    float carrier_shape;
    float carrier_bleed;
    float new_carrier_shape;
    float new_carrier_bleed;
    float sample_rate;
} Aud_GrainletOscillator;

void  Aud_GrainletOscillator_Init(Aud_GrainletOscillator *self, float sample_rate);
float Aud_GrainletOscillator_Process(Aud_GrainletOscillator *self);
void  Aud_GrainletOscillator_SetFreq(Aud_GrainletOscillator *self, float freq);
void  Aud_GrainletOscillator_SetFormantFreq(Aud_GrainletOscillator *self, float freq);
void  Aud_GrainletOscillator_SetShape(Aud_GrainletOscillator *self, float shape);
void  Aud_GrainletOscillator_SetBleed(Aud_GrainletOscillator *self, float bleed);

#endif /* AUD_GRAINLET_H */
