#include "aud_mpu_fx_chain.h"

#include "aud_dsp.h"

#include <stddef.h>

static float aud_mpu_param(float value, float low, float high)
{
    if (!(value == value)) {
        return low;
    }
    return aud_fclamp(value, low, high);
}

void AudMpuFxParams_Default(AudMpuFxParams *params)
{
    if (params == NULL) {
        return;
    }
    params->input_gain = 1.0f;
    params->filter_cutoff_hz = 18000.0f;
    params->filter_resonance = 0.05f;
    params->filter_mix = 0.0f;
    params->drive = 0.0f;
    params->delay_ms = 250.0f;
    params->delay_feedback = 0.25f;
    params->delay_crossfeed = 0.0f;
    params->delay_mix = 0.0f;
    params->output_gain = 0.8f;
}

int AudMpuFxChain_Init(AudMpuFxChain *chain,
                       float sample_rate,
                       float *delay_left,
                       float *delay_right,
                       uint32_t delay_samples)
{
    AudMpuFxParams defaults;
    uint32_t channel;

    if (chain == NULL || delay_left == NULL || delay_right == NULL ||
        sample_rate < 8000.0f || delay_samples < 2u) {
        return -1;
    }

    chain->sample_rate = sample_rate;
    for (channel = 0u; channel < 2u; ++channel) {
        Aud_DcBlock_Init(&chain->dc[channel], sample_rate);
        Aud_Svf_Init(&chain->filter[channel], sample_rate);
        Aud_Overdrive_Init(&chain->drive[channel]);
    }
    Aud_DelayLine_Init(&chain->delay[0], delay_left, delay_samples);
    Aud_DelayLine_Init(&chain->delay[1], delay_right, delay_samples);
    AudMpuFxParams_Default(&defaults);
    AudMpuFxChain_SetParams(chain, &defaults);
    return 0;
}

void AudMpuFxChain_SetParams(AudMpuFxChain *chain,
                             const AudMpuFxParams *params)
{
    float delay_samples;
    float max_delay_ms;
    uint32_t channel;

    if (chain == NULL || params == NULL || chain->sample_rate <= 0.0f) {
        return;
    }

    chain->params.input_gain = aud_mpu_param(params->input_gain, 0.0f, 4.0f);
    chain->params.filter_cutoff_hz = aud_mpu_param(
        params->filter_cutoff_hz, 20.0f, chain->sample_rate / 3.0f);
    chain->params.filter_resonance = aud_mpu_param(
        params->filter_resonance, 0.0f, 1.0f);
    chain->params.filter_mix = aud_mpu_param(params->filter_mix, 0.0f, 1.0f);
    chain->params.drive = aud_mpu_param(params->drive, 0.0f, 1.0f);
    chain->params.delay_feedback = aud_mpu_param(
        params->delay_feedback, -0.95f, 0.95f);
    chain->params.delay_crossfeed = aud_mpu_param(
        params->delay_crossfeed, 0.0f, 1.0f);
    chain->params.delay_mix = aud_mpu_param(params->delay_mix, 0.0f, 1.0f);
    chain->params.output_gain = aud_mpu_param(params->output_gain, 0.0f, 2.0f);

    max_delay_ms = 1000.0f *
        ((float)(chain->delay[0].max_size - 1u) / chain->sample_rate);
    chain->params.delay_ms = aud_mpu_param(params->delay_ms, 0.0f, max_delay_ms);
    delay_samples = chain->params.delay_ms * 0.001f * chain->sample_rate;

    for (channel = 0u; channel < 2u; ++channel) {
        Aud_Svf_SetFreq(&chain->filter[channel], chain->params.filter_cutoff_hz);
        Aud_Svf_SetRes(&chain->filter[channel], chain->params.filter_resonance);
        Aud_Overdrive_SetDrive(&chain->drive[channel], chain->params.drive);
        Aud_DelayLine_SetDelay(&chain->delay[channel], delay_samples);
    }
}

void AudMpuFxChain_Process(void *user,
                           float *interleaved_stereo,
                           uint32_t frames)
{
    AudMpuFxChain *chain = (AudMpuFxChain *)user;
    uint32_t frame;

    if (chain == NULL || interleaved_stereo == NULL) {
        return;
    }

    for (frame = 0u; frame < frames; ++frame) {
        float driven[2];
        float wet[2];
        uint32_t channel;

        for (channel = 0u; channel < 2u; ++channel) {
            uint32_t index = frame * 2u + channel;
            float input = interleaved_stereo[index] * chain->params.input_gain;
            float dry = Aud_DcBlock_Process(&chain->dc[channel], input);
            float filtered;
            Aud_Svf_Process(&chain->filter[channel], dry);
            filtered = dry + (Aud_Svf_Low(&chain->filter[channel]) - dry) *
                chain->params.filter_mix;
            driven[channel] = Aud_Overdrive_Process(&chain->drive[channel],
                                                     filtered);
            wet[channel] = Aud_DelayLine_Read(&chain->delay[channel]);
        }

        Aud_DelayLine_Write(&chain->delay[0], driven[0] +
            (wet[0] * (1.0f - chain->params.delay_crossfeed) +
             wet[1] * chain->params.delay_crossfeed) *
            chain->params.delay_feedback);
        Aud_DelayLine_Write(&chain->delay[1], driven[1] +
            (wet[1] * (1.0f - chain->params.delay_crossfeed) +
             wet[0] * chain->params.delay_crossfeed) *
            chain->params.delay_feedback);

        for (channel = 0u; channel < 2u; ++channel) {
            uint32_t index = frame * 2u + channel;
            float output = driven[channel] + (wet[channel] - driven[channel]) *
                chain->params.delay_mix;
            output = aud_soft_clip(output * chain->params.output_gain);
            aud_test_float(&output, 0.0f);
            interleaved_stereo[index] = output;
        }
    }
}
