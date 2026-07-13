#include <stddef.h>
#include "aud_dcblock.h"

void Aud_DcBlock_Init(Aud_DcBlock *dc, float sample_rate)
{
    (void)sample_rate;
    if (dc == NULL) return;
    dc->input  = 0.0f;
    dc->output = 0.0f;
    dc->gain   = 0.99f;
}

float Aud_DcBlock_Process(Aud_DcBlock *dc, float in)
{
    if (dc == NULL) return in;
    float out = in - dc->input + dc->gain * dc->output;
    dc->output = out;
    dc->input  = in;
    return out;
}
