#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "aud_mpu_fx_chain.h"

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            fprintf(stderr, "CHECK failed at line %d: %s\n",                \
                    __LINE__, #condition);                                     \
            return 1;                                                          \
        }                                                                      \
    } while (0)

int main(void)
{
    float delay_left[128];
    float delay_right[128];
    float block[64] = {0};
    AudMpuFxChain fx;
    AudMpuFxParams params;
    uint32_t i;

    CHECK(AudMpuFxChain_Init(&fx, 48000.0f, delay_left, delay_right, 128u) == 0);
    AudMpuFxParams_Default(&params);
    params.input_gain = 100.0f;
    params.delay_feedback = 100.0f;
    params.delay_crossfeed = 1.0f;
    params.delay_mix = 1.0f;
    params.delay_ms = 1.0f;
    AudMpuFxChain_SetParams(&fx, &params);
    CHECK(fx.params.input_gain == 4.0f);
    CHECK(fx.params.delay_feedback == 0.95f);
    block[0] = 1.0f;
    AudMpuFxChain_Process(&fx, block, 32u);
    for (i = 0u; i < 64u; ++i) {
        CHECK(isfinite(block[i]));
        CHECK(block[i] >= -1.0f && block[i] <= 1.0f);
    }
    puts("MPU_FX_TEST_OK");
    return 0;
}
