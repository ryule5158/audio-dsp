#include <stddef.h>
#include "aud_moogladder.h"
#include "aud_dsp.h"
#include <math.h>

void Aud_MoogLadder_Init(Aud_MoogLadder *ml, float sample_rate)
{
    if (ml == NULL) return;
    ml->sample_rate = sample_rate;
    ml->cutoff      = 1000.0f;
    ml->res         = 0.4f;
    ml->stage[0] = ml->stage[1] = ml->stage[2] = ml->stage[3] = 0.0f;
    ml->p0 = 0.0f;
}

void Aud_MoogLadder_SetFreq(Aud_MoogLadder *ml, float freq)
{
    if (ml == NULL) return;
    ml->cutoff = aud_fclamp(freq, 5.0f, ml->sample_rate * 0.45f);
}

void Aud_MoogLadder_SetRes(Aud_MoogLadder *ml, float res)
{
    if (ml == NULL) return;
    ml->res = aud_fclamp(res, 0.0f, 1.0f);
}

float Aud_MoogLadder_Process(Aud_MoogLadder *ml, float in)
{
    if (ml == NULL) return in;
    float fc  = ml->cutoff;
    float res = ml->res;
    float wc  = AUD_TWOPI * fc / ml->sample_rate;
    float g   = 1.0f - expf(-wc); /* Tustin approximation */
    float r   = 4.0f * res;

    /* compute feedback */
    float fb = ml->stage[3] - in;
    fb *= r;

    /* 4 cascaded one-pole stages */
    float s0 = ml->stage[0] + g * (tanhf(in + fb) - ml->stage[0]);
    float s1 = ml->stage[1] + g * (s0 - ml->stage[1]);
    float s2 = ml->stage[2] + g * (s1 - ml->stage[2]);
    float s3 = ml->stage[3] + g * (s2 - ml->stage[3]);

    ml->stage[0] = s0;
    ml->stage[1] = s1;
    ml->stage[2] = s2;
    ml->stage[3] = s3;

    return s3;
}
