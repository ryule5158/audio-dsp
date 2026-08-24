/**
 * @file aud_mpu_fx_chain.h
 * @brief Portable stereo effect chain tuned for i.MX6ULL Linux.
 */
#ifndef AUD_MPU_FX_CHAIN_H
#define AUD_MPU_FX_CHAIN_H

#include <stdint.h>

#include "aud_dcblock.h"
#include "aud_delayline.h"
#include "aud_overdrive.h"
#include "aud_svf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float input_gain;
    float filter_cutoff_hz;
    float filter_resonance;
    float filter_mix;
    /** 0 is a true bypass; 0..1 also crossfades into the driven signal. */
    float drive;
    float delay_ms;
    float delay_feedback;
    float delay_crossfeed;
    float delay_mix;
    float output_gain;
} AudMpuFxParams;

typedef struct {
    float sample_rate;
    AudMpuFxParams params;
    Aud_DcBlock dc[2];
    Aud_Svf filter[2];
    Aud_Overdrive drive[2];
    Aud_DelayLine delay[2];
} AudMpuFxChain;

void AudMpuFxParams_Default(AudMpuFxParams *params);

int AudMpuFxChain_Init(AudMpuFxChain *chain,
                       float sample_rate,
                       float *delay_left,
                       float *delay_right,
                       uint32_t delay_samples);

/** Apply a complete parameter snapshot before processing the next block. */
void AudMpuFxChain_SetParams(AudMpuFxChain *chain,
                             const AudMpuFxParams *params);

/** Suitable as AudMpuProcessBlockFn. */
void AudMpuFxChain_Process(void *user,
                           float *interleaved_stereo,
                           uint32_t frames);

#ifdef __cplusplus
}
#endif

#endif
