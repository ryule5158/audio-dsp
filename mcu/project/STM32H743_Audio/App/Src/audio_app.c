#include "audio_app.h"

#include "audio_dsp.h"
#include "board_config.h"
#include "i2c.h"
#include "sai.h"
#include "wm8960.h"

#include <stddef.h>

#ifndef AUDIO_BOARD_ENABLE_GENERIC_DSP
#define AUDIO_BOARD_ENABLE_GENERIC_DSP 0u
#endif

#if AUDIO_BOARD_ENABLE_GENERIC_DSP
#include "generic_dsp_selftest.h"
#endif

#if AUDIO_BOARD_CHANNELS != 2u
#error "This reference application requires stereo audio"
#endif

#if AUDIO_BOARD_BITS_PER_SAMPLE != 24u
#error "This reference application requires 24-bit SAI samples"
#endif

#if AUDIO_BOARD_SAMPLE_RATE_HZ != 48000u
#error "Update PLL3, WM8960 clocks and delay length before changing sample rate"
#endif

#define AUDIO_DMA_HALF_SAMPLES \
    (AUDIO_BOARD_DMA_HALF_FRAMES * AUDIO_BOARD_CHANNELS)
#define AUDIO_DMA_TOTAL_SAMPLES (AUDIO_DMA_HALF_SAMPLES * 2u)
#define AUDIO_DELAY_FRAMES \
    ((AUDIO_BOARD_SAMPLE_RATE_HZ * AUDIO_BOARD_DELAY_TIME_MS) / 1000u)
#define AUDIO_DELAY_STORAGE_SAMPLES (AUDIO_DELAY_FRAMES + 1u)
#define AUDIO_SELF_TEST_FRAMES       4u
#define AUDIO_SELF_TEST_SAMPLES \
    (AUDIO_SELF_TEST_FRAMES * AUDIO_BOARD_CHANNELS)

#if AUDIO_DMA_TOTAL_SAMPLES > 65535u
#error "HAL SAI DMA length is limited to uint16_t"
#endif

/* DMA2 reaches D2 SRAM. The scatter file places these buffers in the same
 * 32 KiB non-cacheable MPU region declared by board_config.h and main.c. */
__attribute__((section(".audio_dma"), aligned(32)))
static int32_t s_rx[AUDIO_DMA_TOTAL_SAMPLES];

__attribute__((section(".audio_dma"), aligned(32)))
static int32_t s_tx[AUDIO_DMA_TOTAL_SAMPLES];

static float s_work[AUDIO_DMA_HALF_SAMPLES];
/* One extra ring entry permits an exact 4800-sample (100 ms) delay because
 * Aud_DelayLine reserves max_size - 1 as its greatest valid delay. */
static float s_delay_left[AUDIO_DELAY_STORAGE_SAMPLES];
static float s_delay_right[AUDIO_DELAY_STORAGE_SAMPLES];
static AudioDsp s_dsp;
static Wm8960 s_codec;

volatile int32_t g_audio_app_status = AUDIO_APP_BAD_CONFIG;
volatile uint32_t g_audio_callback_errors = 0u;
volatile uint32_t g_audio_restart_requested = 0u;

static void AudioApp_ClearDmaBuffers(void)
{
    uint32_t sample;

    for (sample = 0u; sample < AUDIO_DMA_TOTAL_SAMPLES; ++sample) {
        s_rx[sample] = 0;
        s_tx[sample] = 0;
    }
}

static void AudioApp_StopDmaStream(void)
{
    (void)HAL_SAI_DMAStop(&hsai_BlockA1);
    (void)HAL_SAI_DMAStop(&hsai_BlockB1);
}

/* Start RX/TX/unmute as one transaction.  Every failure path stops both SAI
 * blocks and requests codec mute so callers never inherit a half-started
 * stream.  The codec has already been initialized before this is called. */
static AudioAppStatus AudioApp_StartDmaStream(void)
{
    HAL_StatusTypeDef status;

    status = HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)s_rx,
                                 (uint16_t)AUDIO_DMA_TOTAL_SAMPLES);
    if (status != HAL_OK) {
        AudioApp_StopDmaStream();
        (void)Wm8960_SetMute(&s_codec, true);
        g_audio_app_status = AUDIO_APP_DMA_FAILED;
        return AUDIO_APP_DMA_FAILED;
    }

    status = HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)s_tx,
                                  (uint16_t)AUDIO_DMA_TOTAL_SAMPLES);
    if (status != HAL_OK) {
        AudioApp_StopDmaStream();
        (void)Wm8960_SetMute(&s_codec, true);
        g_audio_app_status = AUDIO_APP_DMA_FAILED;
        return AUDIO_APP_DMA_FAILED;
    }

    if (Wm8960_SetMute(&s_codec, false) != HAL_OK) {
        AudioApp_StopDmaStream();
        (void)Wm8960_SetMute(&s_codec, true);
        g_audio_app_status = AUDIO_APP_CODEC_FAILED;
        return AUDIO_APP_CODEC_FAILED;
    }

    g_audio_app_status = AUDIO_APP_OK;
    return AUDIO_APP_OK;
}

static void AudioApp_ProcessHalf(uint32_t sample_offset)
{
    if (AudioDsp_ProcessPcm(&s_dsp, s_work, AUDIO_DMA_HALF_SAMPLES,
                            &s_rx[sample_offset], &s_tx[sample_offset],
                            AUDIO_BOARD_DMA_HALF_FRAMES) != 0) {
        ++g_audio_callback_errors;
        g_audio_app_status = AUDIO_APP_BAD_CONFIG;
    }
}

int AudioApp_RunSelfTest(void)
{
    float delay_left[AUDIO_SELF_TEST_SAMPLES + 1u];
    float delay_right[AUDIO_SELF_TEST_SAMPLES + 1u];
    float work[AUDIO_SELF_TEST_SAMPLES];
    AudioDsp test_dsp;
    int32_t rx[AUDIO_SELF_TEST_SAMPLES] = {
        0x00400000, 0x00c00000, 0x00200000, 0x00e00000,
        0x00100000, 0x00f00000, 0x00080000, 0x00f80000
    };
    int32_t tx[AUDIO_SELF_TEST_SAMPLES] = {0};
    float positive;
    float negative;
    uint32_t i;
    int saw_nonzero = 0;
    Aud_DcBlock dc;
    Aud_Osc osc;
    float dc_last = 0.0f;

    positive = AudioDsp_S24ToFloat(0x007fffff);
    negative = AudioDsp_S24ToFloat(0x00800000);
    if ((positive < 0.999f) || (positive >= 1.0f) ||
        (negative > -0.999f) ||
        (AudioDsp_FloatToS24(0.0f) != 0) ||
        (AudioDsp_FloatToS24(-1.0f) != 0x00800000)) {
        return -1;
    }

    if (AudioDsp_Init(&test_dsp, delay_left, delay_right,
                      AUDIO_SELF_TEST_SAMPLES + 1u) != 0) {
        return -1;
    }
    if (AudioDsp_ProcessPcm(&test_dsp, work, AUDIO_SELF_TEST_SAMPLES,
                            rx, tx, AUDIO_SELF_TEST_FRAMES) != 0) {
        return -1;
    }
    if ((test_dsp.processed_blocks != 1u) ||
        ((tx[0] & 0x00ffffff) == 0)) {
        return -1;
    }
    for (i = 0u; i < AUDIO_SELF_TEST_FRAMES; ++i) {
        const float left = AudioDsp_S24ToFloat(tx[i * 2u]);
        const float right = AudioDsp_S24ToFloat(tx[(i * 2u) + 1u]);
        if ((left < -1.0f) || (left > 1.0f) ||
            (right < -1.0f) || (right > 1.0f) ||
            (left <= 0.001f) || (right >= -0.001f) ||
            ((left + right > 0.0001f) ||
             (left + right < -0.0001f))) {
            return -1;
        }
    }

    /* Exercise the actual permissive root modules, not a shadow filter. */
    Aud_DcBlock_Init(&dc, 48000.0f);
    for (i = 0u; i < 128u; ++i) {
        dc_last = Aud_DcBlock_Process(&dc, 0.25f);
    }
    if (dc_last > 0.20f || dc_last < -0.20f) {
        return -1;
    }
    Aud_Osc_Init(&osc, 48000.0f);
    Aud_Osc_SetFreq(&osc, 1000.0f);
    Aud_Osc_SetWaveform(&osc, AUD_WAVE_POLYBLEP_SAW);
    for (i = 0u; i < 48u; ++i) {
        const float sample = Aud_Osc_Process(&osc);
        if ((sample < -1.1f) || (sample > 1.1f)) {
            return -1;
        }
        if (sample > 0.01f || sample < -0.01f) {
            saw_nonzero = 1;
        }
    }
    if (!saw_nonzero) {
        return -1;
    }
    return 0;
}

AudioAppStatus AudioApp_StartHardware(void)
{
    HAL_StatusTypeDef status;

    AudioApp_ClearDmaBuffers();
    status = Wm8960_Init(&s_codec, &hi2c1,
                         AUDIO_BOARD_WM8960_ADDR_7BIT,
                         AUDIO_BOARD_WM8960_I2C_TIMEOUT_MS);
    if (status != HAL_OK) {
        g_audio_app_status = AUDIO_APP_CODEC_FAILED;
        return AUDIO_APP_CODEC_FAILED;
    }

    /* Arm synchronous RX before starting the clock-producing TX block. */
    return AudioApp_StartDmaStream();
}

AudioAppStatus AudioApp_Init(void)
{
    g_audio_callback_errors = 0u;
    g_audio_restart_requested = 0u;
    AudioApp_ClearDmaBuffers();
    if (AudioApp_RunSelfTest() != 0) {
        g_audio_app_status = AUDIO_APP_DSP_SELF_TEST_FAILED;
        return AUDIO_APP_DSP_SELF_TEST_FAILED;
    }
#if AUDIO_BOARD_ENABLE_GENERIC_DSP
    /* Generic DSP is a startup/service operation, never a SAI callback. */
    if (GenericDsp_RunSelfTest() != 0) {
        g_audio_app_status = AUDIO_APP_GENERIC_DSP_SELF_TEST_FAILED;
        return AUDIO_APP_GENERIC_DSP_SELF_TEST_FAILED;
    }
#endif
    if (AudioDsp_Init(&s_dsp, s_delay_left, s_delay_right,
                      AUDIO_DELAY_STORAGE_SAMPLES) != 0) {
        g_audio_app_status = AUDIO_APP_BAD_CONFIG;
        return AUDIO_APP_BAD_CONFIG;
    }

#if AUDIO_BOARD_ENABLE_WM8960_STREAM
    return AudioApp_StartHardware();
#else
    g_audio_app_status = AUDIO_APP_OK;
    return AUDIO_APP_OK;
#endif
}

AudioAppStatus AudioApp_Service(void)
{
#if AUDIO_BOARD_ENABLE_WM8960_STREAM
    AudioDspParams active_params;

    if (g_audio_restart_requested == 0u) {
        return (AudioAppStatus)g_audio_app_status;
    }
    g_audio_restart_requested = 0u;
    if (Wm8960_SetMute(&s_codec, true) != HAL_OK) {
        AudioApp_StopDmaStream();
        g_audio_app_status = AUDIO_APP_CODEC_FAILED;
        return AUDIO_APP_CODEC_FAILED;
    }
    AudioApp_StopDmaStream();
    AudioApp_ClearDmaBuffers();
    active_params = s_dsp.params;
    if (AudioDsp_Init(&s_dsp, s_delay_left, s_delay_right,
                      AUDIO_DELAY_STORAGE_SAMPLES) != 0) {
        g_audio_app_status = AUDIO_APP_BAD_CONFIG;
        return AUDIO_APP_BAD_CONFIG;
    }
    AudioDsp_SetParams(&s_dsp, &active_params);
    return AudioApp_StartDmaStream();
#endif
    return (AudioAppStatus)g_audio_app_status;
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    if ((hsai != NULL) && (hsai->Instance == SAI1_Block_B)) {
        AudioApp_ProcessHalf(0u);
    }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if ((hsai != NULL) && (hsai->Instance == SAI1_Block_B)) {
        AudioApp_ProcessHalf(AUDIO_DMA_HALF_SAMPLES);
    }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
    if ((hsai != NULL) &&
        ((hsai->Instance == SAI1_Block_A) ||
         (hsai->Instance == SAI1_Block_B))) {
        ++g_audio_callback_errors;
        g_audio_app_status = AUDIO_APP_DMA_FAILED;
        g_audio_restart_requested = 1u;
    }
}
