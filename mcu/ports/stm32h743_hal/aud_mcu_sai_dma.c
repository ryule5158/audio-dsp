#include "aud_mcu_sai_dma.h"

#include <stddef.h>

#define AUD_MCU_CACHE_LINE_BYTES 32u

static void aud_mcu_cache_invalidate(void *address, uint32_t bytes)
{
    uintptr_t begin = (uintptr_t)address & ~(uintptr_t)(AUD_MCU_CACHE_LINE_BYTES - 1u);
    uintptr_t end = ((uintptr_t)address + bytes + AUD_MCU_CACHE_LINE_BYTES - 1u) &
        ~(uintptr_t)(AUD_MCU_CACHE_LINE_BYTES - 1u);
    SCB_InvalidateDCache_by_Addr((uint32_t *)begin, (int32_t)(end - begin));
}

static void aud_mcu_cache_clean(void *address, uint32_t bytes)
{
    uintptr_t begin = (uintptr_t)address & ~(uintptr_t)(AUD_MCU_CACHE_LINE_BYTES - 1u);
    uintptr_t end = ((uintptr_t)address + bytes + AUD_MCU_CACHE_LINE_BYTES - 1u) &
        ~(uintptr_t)(AUD_MCU_CACHE_LINE_BYTES - 1u);
    SCB_CleanDCache_by_Addr((uint32_t *)begin, (int32_t)(end - begin));
}

static void aud_mcu_sai_process_half(AudMcuSaiDma *port, uint32_t half)
{
    uint32_t offset;
    uint32_t bytes;
    int32_t *rx;
    int32_t *tx;

    if (port == NULL || port->audio == NULL || half > 1u) {
        return;
    }

    offset = half * port->half_frames * 2u;
    bytes = port->half_frames * 2u * (uint32_t)sizeof(int32_t);
    rx = &port->rx_buffer[offset];
    tx = &port->tx_buffer[offset];

    aud_mcu_cache_invalidate(rx, bytes);
    if (AudMcuAudio_ProcessS32(port->audio, rx, tx, port->half_frames) != 0) {
        port->callback_errors++;
    }
    aud_mcu_cache_clean(tx, bytes);
}

int AudMcuSaiDma_Init(AudMcuSaiDma *port,
                      SAI_HandleTypeDef *rx_sai,
                      SAI_HandleTypeDef *tx_sai,
                      AudMcuAudio *audio,
                      int32_t *rx_buffer,
                      int32_t *tx_buffer,
                      uint32_t half_frames)
{
    if (port == NULL || rx_sai == NULL || tx_sai == NULL || audio == NULL ||
        rx_buffer == NULL || tx_buffer == NULL || half_frames == 0u ||
        half_frames > audio->max_frames ||
        (((uintptr_t)rx_buffer | (uintptr_t)tx_buffer) &
         (AUD_MCU_CACHE_LINE_BYTES - 1u)) != 0u ||
        ((half_frames * 2u * (uint32_t)sizeof(int32_t)) &
         (AUD_MCU_CACHE_LINE_BYTES - 1u)) != 0u) {
        return -1;
    }

    port->rx_sai = rx_sai;
    port->tx_sai = tx_sai;
    port->audio = audio;
    port->rx_buffer = rx_buffer;
    port->tx_buffer = tx_buffer;
    port->half_frames = half_frames;
    port->callback_errors = 0u;
    return 0;
}

int AudMcuSaiDma_Start(AudMcuSaiDma *port)
{
    uint32_t sample_words;
    uint32_t bytes;

    if (port == NULL || port->rx_sai == NULL || port->tx_sai == NULL) {
        return -1;
    }

    sample_words = port->half_frames * 4u;
    if (sample_words > 65535u) {
        return -1;
    }
    bytes = sample_words * (uint32_t)sizeof(int32_t);
    for (uint32_t i = 0u; i < sample_words; ++i) {
        port->tx_buffer[i] = 0;
    }
    aud_mcu_cache_clean(port->tx_buffer, bytes);
    aud_mcu_cache_invalidate(port->rx_buffer, bytes);

    /* Arm RX first. TX master then starts BCLK/LRCK/MCLK. */
    if (HAL_SAI_Receive_DMA(port->rx_sai,
                            (uint8_t *)port->rx_buffer,
                            (uint16_t)sample_words) != HAL_OK) {
        return -1;
    }
    if (HAL_SAI_Transmit_DMA(port->tx_sai,
                             (uint8_t *)port->tx_buffer,
                             (uint16_t)sample_words) != HAL_OK) {
        (void)HAL_SAI_DMAStop(port->rx_sai);
        return -1;
    }
    return 0;
}

void AudMcuSaiDma_OnRxHalfComplete(AudMcuSaiDma *port)
{
    aud_mcu_sai_process_half(port, 0u);
}

void AudMcuSaiDma_OnRxComplete(AudMcuSaiDma *port)
{
    aud_mcu_sai_process_half(port, 1u);
}
