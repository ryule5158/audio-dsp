/**
 * @file aud_mcu_fx_chain.h
 * @brief Small, allocation-free stereo synth/effect chain for STM32H743.
 */
#ifndef AUD_MCU_FX_CHAIN_H
#define AUD_MCU_FX_CHAIN_H

#include <stdint.h>

#include "aud_dcblock.h"
#include "aud_delayline.h"
#include "aud_osc.h"
#include "aud_overdrive.h"
#include "aud_svf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float input_gain;
    float synth_mix;
    float synth_frequency_hz;
    uint8_t synth_waveform;
    float filter_cutoff_hz;
    float filter_resonance;
    float filter_mix;
    float drive;
    float delay_ms;
    float delay_feedback;
    float delay_mix;
    float output_gain;
} AudMcuFxParams;

typedef struct {
    float sample_rate;
    AudMcuFxParams params;
    Aud_DcBlock dc[2];
    Aud_Svf filter[2];
    Aud_Overdrive drive[2];
    Aud_DelayLine delay[2];
    Aud_Osc oscillator;
} AudMcuFxChain;

void AudMcuFxParams_Default(AudMcuFxParams *params);

int AudMcuFxChain_Init(AudMcuFxChain *chain,
                       float sample_rate,
                       float *delay_left,
                       float *delay_right,
                       uint32_t delay_samples);

/** Clamp and apply a caller-owned snapshot; call only at a block boundary. */
void AudMcuFxChain_SetParams(AudMcuFxChain *chain,
                             const AudMcuFxParams *params);

/** Suitable as AudMcuProcessBlockFn. */
void AudMcuFxChain_Process(void *user,
                           float *interleaved_stereo,
                           uint32_t frames);

#ifdef __cplusplus
}
#endif

#endif
