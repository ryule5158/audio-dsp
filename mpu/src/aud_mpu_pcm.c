#include "aud_mpu_pcm.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

static float aud_mpu_s16_to_float(int16_t sample)
{
    return (float)sample / 32768.0f;
}

static float aud_mpu_s32_to_float(int32_t sample)
{
    return (float)sample / 2147483648.0f;
}

static int16_t aud_mpu_float_to_s16(float sample)
{
    if (!(sample == sample)) {
        return 0;
    }
    if (sample <= -1.0f) {
        return INT16_MIN;
    }
    if (sample >= 1.0f) {
        return INT16_MAX;
    }
    return (int16_t)(sample * 32768.0f);
}

static int32_t aud_mpu_float_to_s32(float sample)
{
    if (!(sample == sample)) {
        return 0;
    }
    if (sample <= -1.0f) {
        return INT32_MIN;
    }
    if (sample >= 1.0f) {
        return INT32_MAX;
    }
    return (int32_t)(sample * 2147483648.0f);
}

int AudMpuPcm_Init(AudMpuPcm *pcm,
                   float *work,
                   uint32_t work_samples,
                   uint32_t max_frames,
                   AudMpuPcmFormat format,
                   AudMpuProcessBlockFn process,
                   void *process_user)
{
    if (pcm == NULL || work == NULL || max_frames == 0u ||
        work_samples < max_frames * 2u ||
        (format != AUD_MPU_PCM_S16_LE && format != AUD_MPU_PCM_S32_LE)) {
        return -1;
    }

    pcm->work = work;
    pcm->work_samples = work_samples;
    pcm->max_frames = max_frames;
    pcm->format = format;
    pcm->process = process;
    pcm->process_user = process_user;
    pcm->processed_frames = 0u;
    pcm->rejected_blocks = 0u;
    return 0;
}

uint32_t AudMpuPcm_FrameBytes(const AudMpuPcm *pcm)
{
    if (pcm == NULL) {
        return 0u;
    }
    return pcm->format == AUD_MPU_PCM_S16_LE ? 4u : 8u;
}

int AudMpuPcm_Process(AudMpuPcm *pcm,
                      const void *capture,
                      void *playback,
                      uint32_t frames)
{
    uint32_t samples;
    uint32_t i;

    if (pcm == NULL || capture == NULL || playback == NULL || frames == 0u ||
        frames > pcm->max_frames || pcm->work == NULL) {
        if (pcm != NULL) {
            pcm->rejected_blocks++;
        }
        return -1;
    }

    samples = frames * 2u;
    if (pcm->format == AUD_MPU_PCM_S16_LE) {
        const int16_t *input = (const int16_t *)capture;
        for (i = 0u; i < samples; ++i) {
            pcm->work[i] = aud_mpu_s16_to_float(input[i]);
        }
    } else {
        const int32_t *input = (const int32_t *)capture;
        for (i = 0u; i < samples; ++i) {
            pcm->work[i] = aud_mpu_s32_to_float(input[i]);
        }
    }

    if (pcm->process != NULL) {
        pcm->process(pcm->process_user, pcm->work, frames);
    }

    if (pcm->format == AUD_MPU_PCM_S16_LE) {
        int16_t *output = (int16_t *)playback;
        for (i = 0u; i < samples; ++i) {
            output[i] = aud_mpu_float_to_s16(pcm->work[i]);
        }
    } else {
        int32_t *output = (int32_t *)playback;
        for (i = 0u; i < samples; ++i) {
            output[i] = aud_mpu_float_to_s32(pcm->work[i]);
        }
    }

    pcm->processed_frames += frames;
    return 0;
}
