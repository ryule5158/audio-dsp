#include "aud_fm2.h"
#include "aud_dsp.h"
#include <math.h>
#include <stddef.h>

void Aud_Fm2_Init(Aud_Fm2 *fm, float sample_rate)
{
    if (fm == NULL) return;
    fm->sample_rate = sample_rate;
    fm->lfreq  = 440.0f;
    fm->lratio = 2.0f;
    fm->freq   = 440.0f;
    fm->ratio  = 2.0f;
    fm->idx    = 1.0f;
    fm->phase_car = 0.0f;
    fm->phase_mod = 0.0f;
    fm->inc_car   = AUD_TWOPI * fm->freq / sample_rate;
    fm->inc_mod   = fm->inc_car * fm->ratio;
}

float Aud_Fm2_Process(Aud_Fm2 *fm)
{
    if (fm == NULL) return 0.0f;

    if (fm->lratio != fm->ratio || fm->lfreq != fm->freq) {
        fm->lratio = fm->ratio;
        fm->lfreq  = fm->freq;
        fm->inc_car = AUD_TWOPI * fm->freq / fm->sample_rate;
        fm->inc_mod = fm->inc_car * fm->ratio;
    }

    float modval = sinf(fm->phase_mod);
    fm->phase_mod += fm->inc_mod;
    if (fm->phase_mod > AUD_TWOPI) fm->phase_mod -= AUD_TWOPI;

    fm->phase_car += fm->inc_car + modval * fm->idx * AUD_FM2_KIDX_SCALAR;
    if (fm->phase_car > AUD_TWOPI) fm->phase_car -= AUD_TWOPI;

    return sinf(fm->phase_car);
}

void Aud_Fm2_SetFrequency(Aud_Fm2 *fm, float freq)
    { if (fm) fm->freq = fabsf(freq); }
void Aud_Fm2_SetRatio(Aud_Fm2 *fm, float ratio)
    { if (fm) fm->ratio = fabsf(ratio); }
void Aud_Fm2_SetIndex(Aud_Fm2 *fm, float index)
    { if (fm) fm->idx = index * AUD_FM2_KIDX_SCALAR; }
float Aud_Fm2_GetIndex(Aud_Fm2 *fm)
    { return (fm != NULL) ? fm->idx * AUD_FM2_KIDX_SCALAR_RECIP : 0.0f; }
void Aud_Fm2_Reset(Aud_Fm2 *fm)
    { if (fm) { fm->phase_car = 0.0f; fm->phase_mod = 0.0f; } }
