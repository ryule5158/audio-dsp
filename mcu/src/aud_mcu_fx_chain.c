#include "aud_mcu_fx_chain.h"

#include "aud_dsp.h"

static float aud_mcu_param(float value, float low, float high)
{
    if (!(value == value)) {
        return low;
    }
    return aud_fclamp(value, low, high);
}

void AudMcuFxParams_Default(AudMcuFxParams *params)
{
    if (params == NULL) {
        return;
    }
    params->input_gain = 1.0f;
    params->synth_mix = 0.0f;
    params->synth_frequency_hz = 220.0f;
    params->synth_waveform = AUD_WAVE_POLYBLEP_SAW;
    params->filter_cutoff_hz = 18000.0f;
    params->filter_resonance = 0.05f;
    params->filter_mix = 0.0f;
    params->drive = 0.0f;
    params->delay_ms = 250.0f;
    params->delay_feedback = 0.25f;
    params->delay_mix = 0.0f;
    params->output_gain = 0.8f;
}

int AudMcuFxChain_Init(AudMcuFxChain *chain,
                       float sample_rate,
                       float *delay_left,
                       float *delay_right,
                       uint32_t delay_samples)
{
    uint32_t channel;
    AudMcuFxParams defaults;

    if (chain == NULL || delay_left == NULL || delay_right == NULL ||
        delay_samples < 2u || sample_rate < 8000.0f) {
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
    Aud_Osc_Init(&chain->oscillator, sample_rate);

    AudMcuFxParams_Default(&defaults);
    AudMcuFxChain_SetParams(chain, &defaults);
    return 0;
}

void AudMcuFxChain_SetParams(AudMcuFxChain *chain,
                             const AudMcuFxParams *params)
{
    float max_delay_ms;
    float delay_samples;
    uint32_t channel;

    if (chain == NULL || params == NULL || chain->sample_rate <= 0.0f) {
        return;
    }

    chain->params.input_gain = aud_mcu_param(params->input_gain, 0.0f, 4.0f);
    chain->params.synth_mix = aud_mcu_param(params->synth_mix, 0.0f, 1.0f);
    chain->params.synth_frequency_hz = aud_mcu_param(
        params->synth_frequency_hz, 0.0f, 0.45f * chain->sample_rate);
    chain->params.synth_waveform = params->synth_waveform < AUD_WAVE_LAST
        ? params->synth_waveform : (uint8_t)AUD_WAVE_SIN;
    chain->params.filter_cutoff_hz = aud_mcu_param(
        params->filter_cutoff_hz, 20.0f, chain->sample_rate / 3.0f);
    chain->params.filter_resonance = aud_mcu_param(
        params->filter_resonance, 0.0f, 1.0f);
    chain->params.filter_mix = aud_mcu_param(params->filter_mix, 0.0f, 1.0f);
    chain->params.drive = aud_mcu_param(params->drive, 0.0f, 1.0f);
    chain->params.delay_feedback = aud_mcu_param(
        params->delay_feedback, -0.95f, 0.95f);
    chain->params.delay_mix = aud_mcu_param(params->delay_mix, 0.0f, 1.0f);
    chain->params.output_gain = aud_mcu_param(params->output_gain, 0.0f, 2.0f);

    max_delay_ms = 1000.0f *
        ((float)(chain->delay[0].max_size - 1u) / chain->sample_rate);
    chain->params.delay_ms = aud_mcu_param(params->delay_ms, 0.0f, max_delay_ms);
    delay_samples = chain->params.delay_ms * 0.001f * chain->sample_rate;

    Aud_Osc_SetFreq(&chain->oscillator, chain->params.synth_frequency_hz);
    Aud_Osc_SetWaveform(&chain->oscillator, chain->params.synth_waveform);
    for (channel = 0u; channel < 2u; ++channel) {
        Aud_Svf_SetFreq(&chain->filter[channel], chain->params.filter_cutoff_hz);
        Aud_Svf_SetRes(&chain->filter[channel], chain->params.filter_resonance);
        Aud_Overdrive_SetDrive(&chain->drive[channel], chain->params.drive);
        Aud_DelayLine_SetDelay(&chain->delay[channel], delay_samples);
    }
}

void AudMcuFxChain_Process(void *user,
                           float *interleaved_stereo,
                           uint32_t frames)
{
    AudMcuFxChain *chain = (AudMcuFxChain *)user;
    uint32_t frame;

    if (chain == NULL || interleaved_stereo == NULL) {
        return;
    }

    for (frame = 0u; frame < frames; ++frame) {
        float synth = Aud_Osc_Process(&chain->oscillator);
        uint32_t channel;
        for (channel = 0u; channel < 2u; ++channel) {
            uint32_t index = frame * 2u + channel;
            float input = interleaved_stereo[index] * chain->params.input_gain;
            float dry;
            float filtered;
            float driven;
            float delayed;
            float output;

            input += synth * chain->params.synth_mix;
            dry = Aud_DcBlock_Process(&chain->dc[channel], input);
            Aud_Svf_Process(&chain->filter[channel], dry);
            filtered = dry + (Aud_Svf_Low(&chain->filter[channel]) - dry) *
                chain->params.filter_mix;
            driven = Aud_Overdrive_Process(&chain->drive[channel], filtered);
            delayed = Aud_DelayLine_Read(&chain->delay[channel]);
            Aud_DelayLine_Write(&chain->delay[channel],
                driven + delayed * chain->params.delay_feedback);
            output = driven + (delayed - driven) * chain->params.delay_mix;
            output = aud_soft_clip(output * chain->params.output_gain);
            aud_test_float(&output, 0.0f);
            interleaved_stereo[index] = output;
        }
    }
}
