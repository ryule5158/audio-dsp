#include <stddef.h>
#include "aud_metro.h"

void Aud_Metro_Init(Aud_Metro *m, float sample_rate, float freq)
{
    if (m == NULL) return;
    m->sample_rate = sample_rate;
    m->phase       = 0.0f;
    m->freq        = freq;
    m->phase_inc   = freq / sample_rate;
}

void Aud_Metro_SetFreq(Aud_Metro *m, float freq)
{
    if (m == NULL) return;
    m->freq      = freq;
    m->phase_inc = freq / m->sample_rate;
}

uint8_t Aud_Metro_Process(Aud_Metro *m)
{
    if (m == NULL) return 0;
    m->phase += m->phase_inc;
    if (m->phase >= 1.0f) {
        m->phase -= 1.0f;
        return 1;
    }
    return 0;
}
