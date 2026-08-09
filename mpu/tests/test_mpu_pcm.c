#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "aud_mpu_pcm.h"

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            fprintf(stderr, "CHECK failed at line %d: %s\n",                \
                    __LINE__, #condition);                                     \
            return 1;                                                          \
        }                                                                      \
    } while (0)

static void double_gain(void *user, float *samples, uint32_t frames)
{
    uint32_t i;
    (void)user;
    for (i = 0u; i < frames * 2u; ++i) {
        samples[i] *= 2.0f;
    }
}

int main(void)
{
    float work[8];
    int16_t input16[8] = {0, 16384, -16384, INT16_MAX,
                          INT16_MIN, 8192, -8192, 0};
    int16_t output16[8] = {0};
    int32_t input32[8] = {0, 0x20000000, -0x20000000, INT32_MAX,
                          INT32_MIN, 0x10000000, -0x10000000, 0};
    int32_t output32[8] = {0};
    AudMpuPcm pcm;

    CHECK(AudMpuPcm_Init(&pcm, work, 8u, 4u, AUD_MPU_PCM_S16_LE,
                         double_gain, NULL) == 0);
    CHECK(AudMpuPcm_FrameBytes(&pcm) == 4u);
    CHECK(AudMpuPcm_Process(&pcm, input16, output16, 4u) == 0);
    CHECK(output16[1] == INT16_MAX);
    CHECK(output16[2] == INT16_MIN);
    CHECK(output16[3] == INT16_MAX);
    CHECK(output16[4] == INT16_MIN);
    CHECK(pcm.processed_frames == 4u);

    CHECK(AudMpuPcm_Init(&pcm, work, 8u, 4u, AUD_MPU_PCM_S32_LE,
                         double_gain, NULL) == 0);
    CHECK(AudMpuPcm_FrameBytes(&pcm) == 8u);
    CHECK(AudMpuPcm_Process(&pcm, input32, output32, 4u) == 0);
    CHECK(output32[1] == 0x40000000);
    CHECK(output32[2] == (int32_t)0xc0000000u);
    CHECK(output32[3] == INT32_MAX);
    CHECK(output32[4] == INT32_MIN);
    CHECK(AudMpuPcm_Process(&pcm, input32, output32, 5u) == -1);
    CHECK(pcm.rejected_blocks == 1u);
    puts("MPU_PCM_TEST_OK");
    return 0;
}
