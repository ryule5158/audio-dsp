/**
 * @file aud_mpu_config.h
 * @brief Strict runtime configuration for the Linux target.
 */
#ifndef AUD_MPU_CONFIG_H
#define AUD_MPU_CONFIG_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "aud_mpu_fx_chain.h"
#include "aud_mpu_pcm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUD_MPU_DEVICE_NAME_MAX 128u

typedef struct {
    char capture_device[AUD_MPU_DEVICE_NAME_MAX];
    char playback_device[AUD_MPU_DEVICE_NAME_MAX];
    uint32_t sample_rate;
    uint32_t period_frames;
    uint32_t periods;
    AudMpuPcmFormat format;
    uint32_t max_delay_ms;
    int require_duplex_link;
    int realtime_priority;
    int lock_memory;
    int allow_realtime_fallback;
    AudMpuFxParams effects;
} AudMpuAppConfig;

void AudMpuAppConfig_Default(AudMpuAppConfig *config);

/** Load an INI file. Unknown sections/keys and malformed values fail. */
int AudMpuAppConfig_Load(AudMpuAppConfig *config,
                         const char *path,
                         char *error,
                         size_t error_size);

int AudMpuAppConfig_Validate(const AudMpuAppConfig *config,
                             char *error,
                             size_t error_size);

void AudMpuAppConfig_Print(const AudMpuAppConfig *config, FILE *stream);

#ifdef __cplusplus
}
#endif

#endif
