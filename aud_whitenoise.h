/**
 * @file    aud_whitenoise.h
 * @brief   White noise generator (linear congruential).
 *          Ported from DaisySP Noise/whitenoise (MIT)
 */
#ifndef AUD_WHITENOISE_H
#define AUD_WHITENOISE_H
#include <stdint.h>

typedef struct {
    uint32_t seed;
} Aud_WhiteNoise;

void Aud_WhiteNoise_Init(Aud_WhiteNoise *wn, uint32_t seed);
float Aud_WhiteNoise_Process(Aud_WhiteNoise *wn);

#endif /* AUD_WHITENOISE_H */
