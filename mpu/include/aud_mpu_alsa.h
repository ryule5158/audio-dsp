/**
 * @file aud_mpu_alsa.h
 * @brief Blocking ALSA full-duplex runtime for i.MX6ULL/WM8960.
 */
#ifndef AUD_MPU_ALSA_H
#define AUD_MPU_ALSA_H

#include <signal.h>
#include <stdint.h>

#include <alsa/asoundlib.h>

#include "aud_mpu_pcm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *capture_device;
    const char *playback_device;
    uint32_t sample_rate;
    uint32_t period_frames;
    uint32_t periods;
    AudMpuPcmFormat format;
} AudMpuAlsaConfig;

typedef struct {
    uint64_t frames;
    uint32_t capture_xruns;
    uint32_t playback_xruns;
    uint32_t short_reads;
    uint32_t short_writes;
} AudMpuAlsaStats;

typedef struct {
    AudMpuAlsaConfig config;
    AudMpuAlsaStats stats;
    AudMpuPcm *pcm;
    snd_pcm_t *capture;
    snd_pcm_t *playback;
    void *capture_buffer;
    void *playback_buffer;
    uint32_t buffer_bytes;
} AudMpuAlsa;

int AudMpuAlsa_Open(AudMpuAlsa *alsa,
                    const AudMpuAlsaConfig *config,
                    AudMpuPcm *pcm,
                    void *capture_buffer,
                    void *playback_buffer,
                    uint32_t buffer_bytes);

int AudMpuAlsa_Run(AudMpuAlsa *alsa, volatile sig_atomic_t *stop_requested);
void AudMpuAlsa_Close(AudMpuAlsa *alsa);

#ifdef __cplusplus
}
#endif

#endif
