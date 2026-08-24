#ifndef WM8960_H
#define WM8960_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32h7xx_hal.h"

typedef struct {
    I2C_HandleTypeDef *i2c;
    uint16_t address_8bit;
    uint32_t timeout_ms;
    uint16_t control1_shadow;
    bool initialized;
} Wm8960;

HAL_StatusTypeDef Wm8960_Init(Wm8960 *codec, I2C_HandleTypeDef *i2c,
                              uint8_t address_7bit, uint32_t timeout_ms);
HAL_StatusTypeDef Wm8960_SetMute(Wm8960 *codec, bool mute);
HAL_StatusTypeDef Wm8960_WriteRegister(Wm8960 *codec, uint8_t reg,
                                       uint16_t value9);

#endif
