#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "aud_drip.h"
#include "aud_pitchshifter.h"

int main(void)
{
    Aud_Drip drip;
    Aud_PitchShifter shifter;
    float phase_before;
    float sample;

    srand(1u);
    Aud_Drip_Init(&drip, 48000.0f, 1.0f);
    drip.num_tubes = 32767.0f;
    sample = Aud_Drip_Process(&drip, false);
    assert(isfinite(sample));
    assert(drip.outputs10 != 0.0f);
    assert(drip.outputs20 != 0.0f);

    Aud_PitchShifter_Init(&shifter, 48000.0f, 100.0f);
    assert(shifter.shift == 24.0f);
    Aud_PitchShifter_SetShift(&shifter, 12.0f);
    Aud_PitchShifter_SetMix(&shifter, 1.0f);
    phase_before = shifter.phase;
    sample = Aud_PitchShifter_Process(&shifter, 1.0f);
    assert(isfinite(sample));
    assert(shifter.phase != phase_before);
    assert(shifter.phase >= 0.0f && shifter.phase < 1.0f);

    Aud_PitchShifter_SetShift(&shifter, 100.0f);
    assert(shifter.shift == 24.0f);
    Aud_PitchShifter_SetShift(&shifter, -100.0f);
    assert(shifter.shift == -24.0f);
    return 0;
}
