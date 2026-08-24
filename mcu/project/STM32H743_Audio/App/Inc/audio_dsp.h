#ifndef AUDIO_DSP_H
#define AUDIO_DSP_H

#include <stdint.h>

/* The application deliberately uses the repository's portable modules.  The
 * Keil post-generation script adds the complete permissive source manifest to
 * the project; these types are therefore the same API used by MCU/MPU builds. */
#include "aud_dcblock.h"
#include "aud_delayline.h"
#include "aud_osc.h"
#include "aud_overdrive.h"
#include "aud_svf.h"

typedef struct {
    float input_gain;
    float synth_mix;
    float synth_frequency_hz;
    uint8_t synth_waveform;
    float lowpass_mix;
    float drive;
    float delay_time_ms;
    float delay_mix;
    float delay_feedback;
    float output_gain;
} AudioDspParams;

typedef struct {
    float sample_rate;
    Aud_DcBlock dc[2];
    Aud_Svf filter[2];
    Aud_Overdrive overdrive[2];
    Aud_DelayLine delay[2];
    Aud_Osc oscillator;
    AudioDspParams params;
    uint32_t delay_length;
    uint32_t processed_blocks;
} AudioDsp;

void AudioDsp_DefaultParams(AudioDspParams *params);
int AudioDsp_Init(AudioDsp *dsp, float *delay_left, float *delay_right,
                  uint32_t delay_length);
void AudioDsp_SetParams(AudioDsp *dsp, const AudioDspParams *params);
void AudioDsp_Process(AudioDsp *dsp, float *interleaved_stereo,
                      uint32_t frames);

float AudioDsp_S24ToFloat(int32_t value);
int32_t AudioDsp_FloatToS24(float value);
int AudioDsp_ProcessPcm(AudioDsp *dsp, float *work, uint32_t work_samples,
                        const int32_t *rx, int32_t *tx, uint32_t frames);

#endif
