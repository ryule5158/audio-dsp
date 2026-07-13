/**
 * @file    aud_maytrig.h
 * @brief   Probabilistic trigger — returns true with given probability.
 *          Ported from DaisySP Utility/maytrig (MIT)
 *          Original by Paul Batchelor
 */
#ifndef AUD_MAYTRIG_H
#define AUD_MAYTRIG_H
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int _dummy;
} Aud_Maytrig;

static inline void Aud_Maytrig_Init(Aud_Maytrig *self)
{
    (void)self;
}

static inline bool Aud_Maytrig_Process(Aud_Maytrig *self, float prob)
{
    (void)self;
    return ((float)rand() / (float)RAND_MAX) <= prob;
}

#endif /* AUD_MAYTRIG_H */
