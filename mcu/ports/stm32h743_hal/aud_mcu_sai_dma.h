/**
 * @file aud_mcu_sai_dma.h
 * @brief STM32H743 HAL SAI full-duplex DMA adapter.
 */
#ifndef AUD_MCU_SAI_DMA_H
#define AUD_MCU_SAI_DMA_H

#include <stdint.h>

#include "aud_mcu_audio.h"
#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    SAI_HandleTypeDef *rx_sai;
    SAI_HandleTypeDef *tx_sai;
    AudMcuAudio *audio;
    int32_t *rx_buffer;
    int32_t *tx_buffer;
    uint32_t half_frames;
    uint32_t callback_errors;
} AudMcuSaiDma;

int AudMcuSaiDma_Init(AudMcuSaiDma *port,
                      SAI_HandleTypeDef *rx_sai,
                      SAI_HandleTypeDef *tx_sai,
                      AudMcuAudio *audio,
                      int32_t *rx_buffer,
                      int32_t *tx_buffer,
                      uint32_t half_frames);

int AudMcuSaiDma_Start(AudMcuSaiDma *port);
void AudMcuSaiDma_OnRxHalfComplete(AudMcuSaiDma *port);
void AudMcuSaiDma_OnRxComplete(AudMcuSaiDma *port);

#ifdef __cplusplus
}
#endif

#endif
