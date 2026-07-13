/**
 * @file    aud_dust.h
 * @brief   Randomly-clocked impulse generator.
 *          Ported from DaisySP Noise/dust (MIT). Original by Emilie Gillet, 2016.
 */
#ifndef AUD_DUST_H
#define AUD_DUST_H
#include <stdint.h>
#include <stdlib.h>
#include "aud_dsp.h"

typedef struct {
    float density;
} Aud_Dust;

static inline void Aud_Dust_Init(Aud_Dust *self)
{
    if (self == NULL) return;
    self->density = 0.15f; /* SetDensity(0.5f) → 0.5 * 0.3 = 0.15 */
}

static inline float Aud_Dust_Process(Aud_Dust *self)
{
    if (self == NULL) return 0.0f;
    float inv_density = 1.0f / self->density;
    float u = ((float)rand() / (float)RAND_MAX);
    if (u < self->density) {
        return u * inv_density;
    }
    return 0.0f;
}

static inline void Aud_Dust_SetDensity(Aud_Dust *self, float density)
{
    if (self == NULL) return;
    self->density = aud_fclamp(density, 0.0f, 1.0f) * 0.3f;
}

#endif /* AUD_DUST_H */
