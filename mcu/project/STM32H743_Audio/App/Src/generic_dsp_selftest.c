#include "generic_dsp_selftest.h"

#include "Adaptive.h"
#include "FilterEx.h"
#include "Measure.h"
#include "arm_const_structs.h"
#include "arm_math.h"

#include <math.h>
#include <stddef.h>

static int GenericDsp_NearlyEqual(float lhs, float rhs, float tolerance)
{
    return (fabsf(lhs - rhs) <= tolerance) ? 1 : 0;
}

int GenericDsp_RunSelfTest(void)
{
    static const float waveform[8] = {
        -1.0f, -0.5f, 0.0f, 0.5f, 1.0f, 0.5f, 0.0f, -0.5f
    };
    static const float harmonics[3] = {1.0f, 0.1f, 0.05f};
    float dc;
    float rms;
    float acrms;
    float vpp;
    float thd_db;
    float fft_buffer[128] = {0.0f};
    float fft_magnitude[64] = {0.0f};
    Contest_OnlineStats_t stats;
    FilterEx_Biquad_t biquad;
    float biquad_output;

    /* Exercise deterministic time-domain metrics with no heap allocation. */
    Measure_TimeStats(waveform, 8u, &dc, &rms, &acrms, &vpp);
    if (!GenericDsp_NearlyEqual(dc, 0.0f, 1.0e-6f) ||
        !GenericDsp_NearlyEqual(vpp, 2.0f, 1.0e-6f) ||
        !(rms > 0.60f && rms < 0.80f) ||
        !(acrms > 0.60f && acrms < 0.80f)) {
        return -1;
    }

    if (!(Measure_THD(harmonics, 3u, &thd_db) > 0.10f) ||
        !(thd_db < -15.0f && thd_db > -30.0f)) {
        return -1;
    }

    Contest_StatsInit(&stats);
    Contest_StatsPush(&stats, 1.0f);
    Contest_StatsPush(&stats, 2.0f);
    Contest_StatsPush(&stats, 3.0f);
    if (stats.n != 3u ||
        !GenericDsp_NearlyEqual(stats.mean, 2.0f, 1.0e-6f) ||
        !GenericDsp_NearlyEqual(Contest_StatsVariance(&stats), 1.0f,
                                    1.0e-5f)) {
        return -1;
    }

    if (FilterEx_BiquadDesign(&biquad, FILTEREX_BIQUAD_LOWPASS,
                              48000.0f, 2000.0f, 0.707f, 0.0f) !=
            FILTEREX_OK ||
        FilterEx_BiquadIsStable(&biquad) == 0) {
        return -1;
    }
    biquad_output = FilterEx_BiquadProcess(&biquad, 1.0f);
    if (!isfinite(biquad_output) || biquad_output < 0.0f ||
        biquad_output > 1.0f) {
        return -1;
    }

    /* Exercise the installed CMSIS-DSP Pack contract with a small CFFT. */
    fft_buffer[0] = 1.0f;
    arm_cfft_f32(&arm_cfft_sR_f32_len64, fft_buffer, 0u, 1u);
    arm_cmplx_mag_f32(fft_buffer, fft_magnitude, 64u);
    if (!isfinite(fft_magnitude[0]) ||
        !GenericDsp_NearlyEqual(fft_magnitude[0], 1.0f, 1.0e-5f)) {
        return -1;
    }

    return 0;
}
