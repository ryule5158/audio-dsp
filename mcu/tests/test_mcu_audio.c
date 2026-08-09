#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>

#include "aud_mcu_audio.h"

static void gain_half(void *user, float *samples, uint32_t frames)
{
    float gain = *(const float *)user;
    uint32_t i;
    for (i = 0u; i < frames * 2u; ++i) {
        samples[i] *= gain;
    }
}

int main(void)
{
    float work[8];
    float gain = 0.5f;
    int32_t rx[8] = {0, INT32_MAX, INT32_MIN, 0x40000000,
                     -0x40000000, 0x01000000, -0x01000000, 0};
    int32_t tx[8] = {0};
    AudMcuAudio audio;

    assert(fabsf(AudMcuAudio_S32ToFloat(INT32_MIN) + 1.0f) < 1.0e-7f);
    assert(AudMcuAudio_FloatToS32(-1.0f, AUD_MCU_PCM_S32) == INT32_MIN);
    assert(AudMcuAudio_FloatToS32(1.0f, AUD_MCU_PCM_S32) == INT32_MAX);
    assert(AudMcuAudio_FloatToS32(1.0f, AUD_MCU_PCM_S24_LEFT) == 0x7fffff00);
    assert(AudMcuAudio_FloatToS32(NAN, AUD_MCU_PCM_S32) == 0);
    assert((AudMcuAudio_FloatToS32(0.25f, AUD_MCU_PCM_S24_LEFT) & 0xff) == 0);
    assert(AudMcuAudio_Init(&audio, work, 8u, 4u, AUD_MCU_PCM_S24_LEFT,
                            gain_half, &gain) == 0);
    assert(AudMcuAudio_ProcessS32(&audio, rx, tx, 4u) == 0);
    assert(tx[0] == 0);
    assert(tx[2] == (int32_t)0xc0000000u);
    assert(tx[3] == 0x20000000);
    assert(audio.processed_blocks == 1u);
    assert(audio.rejected_blocks == 0u);
    assert(AudMcuAudio_ProcessS32(&audio, rx, tx, 5u) == -1);
    assert(audio.rejected_blocks == 1u);
    return 0;
}
