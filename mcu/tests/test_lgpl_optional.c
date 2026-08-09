#include <math.h>
#include <stddef.h>
#include <stdio.h>

#include "aud_reverbsc.h"

int main(void)
{
    static Aud_ReverbSc reverb;
    float left = 0.0f;
    float right = 0.0f;
    int index;

    if (Aud_ReverbSc_Init(&reverb, 48000.0f) != 0) return 1;
    for (index = 0; index < 8; ++index) {
        ptrdiff_t start = reverb.delay_lines[index].buf - reverb.aux;
        ptrdiff_t end = start + reverb.delay_lines[index].buffer_size;
        if (start < 0 || end > AUD_REVERBSC_MAX_SIZE) return 2;
        if (index > 0 && reverb.delay_lines[index].buf <
            reverb.delay_lines[index - 1].buf +
            reverb.delay_lines[index - 1].buffer_size) return 3;
    }
    if (Aud_ReverbSc_Process(&reverb, 1.0f, -1.0f, &left, &right) != 0)
        return 4;
    if (!isfinite(left) || !isfinite(right)) return 5;
    puts("MCU_LGPL_OPTIONAL_TEST_OK");
    return 0;
}
