/**
 * @file aud_mpu_pcm.h
 * @brief Allocation-free ALSA PCM to floating-point DSP bridge.
 */
#ifndef AUD_MPU_PCM_H
#define AUD_MPU_PCM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AUD_MPU_PCM_S16_LE = 0,
    AUD_MPU_PCM_S32_LE = 1
} AudMpuPcmFormat;

typedef void (*AudMpuProcessBlockFn)(void *user,
                                     float *interleaved_stereo,
                                     uint32_t frames);

typedef struct {
    float *work;
    uint32_t work_samples;
    uint32_t max_frames;
    AudMpuPcmFormat format;
    AudMpuProcessBlockFn process;
    void *process_user;
    uint64_t processed_frames;
    uint32_t rejected_blocks;
} AudMpuPcm;

int AudMpuPcm_Init(AudMpuPcm *pcm,
                   float *work,
                   uint32_t work_samples,
                   uint32_t max_frames,
                   AudMpuPcmFormat format,
                   AudMpuProcessBlockFn process,
                   void *process_user);

uint32_t AudMpuPcm_FrameBytes(const AudMpuPcm *pcm);

int AudMpuPcm_Process(AudMpuPcm *pcm,
                      const void *capture,
                      void *playback,
                      uint32_t frames);

#ifdef __cplusplus
}
#endif

#endif
