#include <stddef.h>
#include "aud_phasor.h"
#include "aud_dsp.h"

void Aud_Phasor_Init(Aud_Phasor *p, float sample_rate,
                     float freq, float initial_phase)
{
    if (p == NULL) return;
    p->sample_rate = sample_rate;
    p->phs         = initial_phase;
    Aud_Phasor_SetFreq(p, freq);
}

void Aud_Phasor_SetFreq(Aud_Phasor *p, float freq)
{
    if (p == NULL) return;
    p->freq = freq;
    p->inc  = AUD_TWOPI * freq / p->sample_rate;
}

float Aud_Phasor_Process(Aud_Phasor *p)
{
    if (p == NULL) return 0.0f;
    float out = p->phs / AUD_TWOPI;
    p->phs += p->inc;
    if (p->phs > AUD_TWOPI)  p->phs -= AUD_TWOPI;
    if (p->phs < 0.0f)       p->phs  = 0.0f;
    return out;
}
