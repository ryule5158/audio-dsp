/**
 * @file    aud_metro.h
 * @brief   Metronome — generates periodic trigger pulses.
 *          Ported from DaisySP Utility/metro (MIT)
 */
#ifndef AUD_METRO_H
#define AUD_METRO_H
#include <stdint.h>

typedef struct {
    float sample_rate;
    float freq;          /* Trigger frequency (Hz) */
    float phase;
    float phase_inc;
} Aud_Metro;

void Aud_Metro_Init(Aud_Metro *m, float sample_rate, float freq);
void Aud_Metro_SetFreq(Aud_Metro *m, float freq);
uint8_t Aud_Metro_Process(Aud_Metro *m);  /* returns 1 on trigger, 0 otherwise */

#endif /* AUD_METRO_H */
