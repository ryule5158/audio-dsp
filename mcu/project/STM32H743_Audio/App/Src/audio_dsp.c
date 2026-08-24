#include "audio_dsp.h"

#include <stddef.h>

#include "aud_dsp.h"
#include "board_config.h"

#define AUDIO_DSP_S24_SCALE 8388608.0f

static float AudioDsp_Clamp(float value, float minimum, float maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

void AudioDsp_DefaultParams(AudioDspParams *params)
{
    if (params == NULL) {
        return;
    }

    params->input_gain = 1.0f;
    params->synth_mix = 0.0f;
    params->synth_frequency_hz = 220.0f;
    params->synth_waveform = AUD_WAVE_POLYBLEP_SAW;
    params->lowpass_mix = 0.0f;
    params->drive = 0.0f;
    params->delay_time_ms = (float)AUDIO_BOARD_DELAY_TIME_MS;
    params->delay_mix = 0.0f;
    params->delay_feedback = 0.25f;
    params->output_gain = 0.80f;
}

int AudioDsp_Init(AudioDsp *dsp, float *delay_left, float *delay_right,
                  uint32_t delay_length)
{
    uint32_t channel;
    AudioDspParams defaults;

    if ((dsp == NULL) || (delay_left == NULL) || (delay_right == NULL) ||
        (delay_length < 2u)) {
        return -1;
    }

    dsp->sample_rate = 48000.0f;
    dsp->delay_length = delay_length;
    dsp->processed_blocks = 0u;
    for (channel = 0u; channel < 2u; ++channel) {
        Aud_DcBlock_Init(&dsp->dc[channel], dsp->sample_rate);
        Aud_Svf_Init(&dsp->filter[channel], dsp->sample_rate);
        Aud_Overdrive_Init(&dsp->overdrive[channel]);
    }
    Aud_DelayLine_Init(&dsp->delay[0], delay_left, delay_length);
    Aud_DelayLine_Init(&dsp->delay[1], delay_right, delay_length);
    Aud_Osc_Init(&dsp->oscillator, dsp->sample_rate);
    AudioDsp_DefaultParams(&defaults);
    AudioDsp_SetParams(dsp, &defaults);
    return 0;
}

void AudioDsp_SetParams(AudioDsp *dsp, const AudioDspParams *params)
{
    if ((dsp == NULL) || (params == NULL)) {
        return;
    }

    dsp->params.input_gain = AudioDsp_Clamp(params->input_gain, 0.0f, 4.0f);
    dsp->params.synth_mix = AudioDsp_Clamp(params->synth_mix, 0.0f, 1.0f);
    dsp->params.synth_frequency_hz = AudioDsp_Clamp(
        params->synth_frequency_hz, 0.0f, 0.45f * dsp->sample_rate);
    dsp->params.synth_waveform = (params->synth_waveform < AUD_WAVE_LAST)
        ? params->synth_waveform : (uint8_t)AUD_WAVE_SIN;
    dsp->params.lowpass_mix = AudioDsp_Clamp(params->lowpass_mix, 0.0f, 1.0f);
    dsp->params.drive = AudioDsp_Clamp(params->drive, 0.0f, 1.0f);
    dsp->params.delay_time_ms = AudioDsp_Clamp(
        params->delay_time_ms, 0.0f,
        ((float)(dsp->delay_length - 1u) * 1000.0f) / dsp->sample_rate);
    dsp->params.delay_mix = AudioDsp_Clamp(params->delay_mix, 0.0f, 1.0f);
    dsp->params.delay_feedback =
        AudioDsp_Clamp(params->delay_feedback, -0.95f, 0.95f);
    dsp->params.output_gain = AudioDsp_Clamp(params->output_gain, 0.0f, 2.0f);

    Aud_Osc_SetFreq(&dsp->oscillator, dsp->params.synth_frequency_hz);
    Aud_Osc_SetWaveform(&dsp->oscillator, dsp->params.synth_waveform);
    for (uint32_t channel = 0u; channel < 2u; ++channel) {
        Aud_Svf_SetFreq(&dsp->filter[channel], 18000.0f);
        Aud_Svf_SetRes(&dsp->filter[channel], 0.05f);
        /* Treat drive=0 as a true bypass.  The portable overdrive's own zero
         * setting has zero pre-gain, so feeding it unconditionally would mute
         * the otherwise dry default signal. */
        Aud_Overdrive_SetDrive(&dsp->overdrive[channel],
                               0.25f + (0.75f * dsp->params.drive));
        Aud_DelayLine_SetDelay(&dsp->delay[channel],
                               dsp->params.delay_time_ms *
                               dsp->sample_rate / 1000.0f);
    }
}

void AudioDsp_Process(AudioDsp *dsp, float *interleaved_stereo,
                      uint32_t frames)
{
    uint32_t frame;

    if ((dsp == NULL) || (interleaved_stereo == NULL)) {
        return;
    }

    for (frame = 0u; frame < frames; ++frame) {
        const float synth = Aud_Osc_Process(&dsp->oscillator) *
                            dsp->params.synth_mix;
        uint32_t channel;

        for (channel = 0u; channel < 2u; ++channel) {
            const uint32_t sample_index = (frame * 2u) + channel;
            float input = interleaved_stereo[sample_index] *
                          dsp->params.input_gain;
            float filtered;
            float driven;
            float delayed;
            float output;

            input += synth;
            input = Aud_DcBlock_Process(&dsp->dc[channel], input);
            Aud_Svf_Process(&dsp->filter[channel], input);
            filtered = input + (Aud_Svf_Low(&dsp->filter[channel]) - input) *
                       dsp->params.lowpass_mix;
            driven = Aud_Overdrive_Process(&dsp->overdrive[channel], filtered);
            driven = filtered + ((driven - filtered) * dsp->params.drive);
            delayed = Aud_DelayLine_Read(&dsp->delay[channel]);
            Aud_DelayLine_Write(&dsp->delay[channel],
                                driven + delayed * dsp->params.delay_feedback);
            output = driven + ((delayed - driven) * dsp->params.delay_mix);
            output = aud_soft_clip(output * dsp->params.output_gain);
            aud_test_float(&output, 0.0f);
            interleaved_stereo[sample_index] =
                AudioDsp_Clamp(output, -0.999999f, 0.999999f);
        }
    }
    ++dsp->processed_blocks;
}

float AudioDsp_S24ToFloat(int32_t value)
{
    uint32_t raw = ((uint32_t)value) & 0x00ffffffu;
    int32_t signed_value;

    if ((raw & 0x00800000u) != 0u) {
        raw |= 0xff000000u;
    }
    signed_value = (int32_t)raw;
    return (float)signed_value / AUDIO_DSP_S24_SCALE;
}

int32_t AudioDsp_FloatToS24(float value)
{
    float limited = AudioDsp_Clamp(value, -1.0f, 0.99999988f);
    int32_t sample = (int32_t)(limited * AUDIO_DSP_S24_SCALE);
    return (int32_t)(((uint32_t)sample) & 0x00ffffffu);
}

int AudioDsp_ProcessPcm(AudioDsp *dsp, float *work, uint32_t work_samples,
                        const int32_t *rx, int32_t *tx, uint32_t frames)
{
    uint32_t sample;
    const uint32_t samples = frames * 2u;

    if ((dsp == NULL) || (work == NULL) || (rx == NULL) || (tx == NULL) ||
        (frames == 0u) || (work_samples < samples)) {
        return -1;
    }

    for (sample = 0u; sample < samples; ++sample) {
        work[sample] = AudioDsp_S24ToFloat(rx[sample]);
    }
    AudioDsp_Process(dsp, work, frames);
    for (sample = 0u; sample < samples; ++sample) {
        tx[sample] = AudioDsp_FloatToS24(work[sample]);
    }
    return 0;
}
