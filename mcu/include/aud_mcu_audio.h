/**
 * @file aud_mcu_audio.h
 * @brief HAL-independent STM32H7 PCM/DSP bridge.
 *
 * The bridge converts stereo samples from SAI DMA buffers to interleaved
 * float, calls a block processor, and converts the result back with
 * saturation. It allocates no memory and performs no I/O.
 */
#ifndef AUD_MCU_AUDIO_H
#define AUD_MCU_AUDIO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    /** Signed Q1.31 / full-scale 32-bit PCM. */
    AUD_MCU_PCM_S32 = 0,
    /** Signed 24-bit PCM left-aligned in a 32-bit SAI slot. */
    AUD_MCU_PCM_S24_LEFT = 1
} AudMcuPcmFormat;

typedef void (*AudMcuProcessBlockFn)(void *user,
                                     float *interleaved_stereo,
                                     uint32_t frames);

typedef struct {
    float *work;
    uint32_t work_samples;
    uint32_t max_frames;
    AudMcuPcmFormat format;
    AudMcuProcessBlockFn process;
    void *process_user;
    uint32_t processed_blocks;
    uint32_t rejected_blocks;
} AudMcuAudio;

/**
 * Initialize a bridge with caller-owned work memory.
 * work_samples must be at least 2 * max_frames.
 */
int AudMcuAudio_Init(AudMcuAudio *audio,
                     float *work,
                     uint32_t work_samples,
                     uint32_t max_frames,
                     AudMcuPcmFormat format,
                     AudMcuProcessBlockFn process,
                     void *process_user);

/** Process one interleaved stereo half-buffer. */
int AudMcuAudio_ProcessS32(AudMcuAudio *audio,
                          const int32_t *rx,
                          int32_t *tx,
                          uint32_t frames);

float AudMcuAudio_S32ToFloat(int32_t sample);
int32_t AudMcuAudio_FloatToS32(float sample, AudMcuPcmFormat format);

#ifdef __cplusplus
}
#endif

#endif
