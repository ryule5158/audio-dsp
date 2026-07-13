/**
 * @file    aud_fractal_noise.h
 * @brief   Fractal noise — stacks octaves of ClockedNoise for rich noise textures.
 *          Ported from DaisySP Noise/fractal_noise (MIT). Original by Emilie Gillet, 2016.
 *          Instantiated with T=ClockedNoise, order=3.
 */
#ifndef AUD_FRACTAL_NOISE_H
#define AUD_FRACTAL_NOISE_H
#include <stdint.h>
#include "aud_clockednoise.h"
#define AUD_FRACTAL_ORDER 3

typedef struct {
    float sample_rate;
    float frequency;
    float decay;
    Aud_ClockedNoise generator[AUD_FRACTAL_ORDER];
} Aud_FractalRandomGenerator;

void  Aud_FractalRandomGenerator_Init(Aud_FractalRandomGenerator *self, float sample_rate);
float Aud_FractalRandomGenerator_Process(Aud_FractalRandomGenerator *self);
void  Aud_FractalRandomGenerator_SetFreq(Aud_FractalRandomGenerator *self, float freq);
void  Aud_FractalRandomGenerator_SetColor(Aud_FractalRandomGenerator *self, float color);

#endif /* AUD_FRACTAL_NOISE_H */
