#include "aud_mcu_audio.h"

#include <limits.h>

#define AUD_MCU_Q31_SCALE       (2147483648.0f)

int AudMcuAudio_Init(AudMcuAudio *audio,
                     float *work,
                     uint32_t work_samples,
                     uint32_t max_frames,
                     AudMcuPcmFormat format,
                     AudMcuProcessBlockFn process,
                     void *process_user)
{
    if (audio == NULL || work == NULL || max_frames == 0u ||
        work_samples < (2u * max_frames) ||
        (format != AUD_MCU_PCM_S32 && format != AUD_MCU_PCM_S24_LEFT)) {
        return -1;
    }

    audio->work = work;
    audio->work_samples = work_samples;
    audio->max_frames = max_frames;
    audio->format = format;
    audio->process = process;
    audio->process_user = process_user;
    audio->processed_blocks = 0u;
    audio->rejected_blocks = 0u;
    return 0;
}

float AudMcuAudio_S32ToFloat(int32_t sample)
{
    return (float)sample / AUD_MCU_Q31_SCALE;
}

int32_t AudMcuAudio_FloatToS32(float sample, AudMcuPcmFormat format)
{
    int32_t encoded;

    if (!(sample == sample)) {
        encoded = 0;
    } else if (sample <= -1.0f) {
        encoded = INT32_MIN;
    } else if (sample >= 1.0f) {
        encoded = INT32_MAX;
    } else {
        encoded = (int32_t)(sample * AUD_MCU_Q31_SCALE);
    }

    if (format == AUD_MCU_PCM_S24_LEFT) {
        encoded = (int32_t)((uint32_t)encoded & 0xffffff00u);
    }
    return encoded;
}

int AudMcuAudio_ProcessS32(AudMcuAudio *audio,
                          const int32_t *rx,
                          int32_t *tx,
                          uint32_t frames)
{
    uint32_t sample_count;
    uint32_t i;

    if (audio == NULL || rx == NULL || tx == NULL || frames == 0u ||
        frames > audio->max_frames || audio->work == NULL) {
        if (audio != NULL) {
            audio->rejected_blocks++;
        }
        return -1;
    }

    sample_count = frames * 2u;
    for (i = 0u; i < sample_count; ++i) {
        audio->work[i] = AudMcuAudio_S32ToFloat(rx[i]);
    }

    if (audio->process != NULL) {
        audio->process(audio->process_user, audio->work, frames);
    }

    for (i = 0u; i < sample_count; ++i) {
        tx[i] = AudMcuAudio_FloatToS32(audio->work[i], audio->format);
    }

    audio->processed_blocks++;
    return 0;
}
