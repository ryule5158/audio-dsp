#include "wm8960.h"

#include <stddef.h>

/* WM8960's control port is write-only and carries a 7-bit register address
 * plus a 9-bit value in two bytes. */
#define WM8960_REG_LEFT_INPUT_VOLUME   0x00u
#define WM8960_REG_RIGHT_INPUT_VOLUME  0x01u
#define WM8960_REG_LEFT_OUTPUT_VOLUME  0x02u
#define WM8960_REG_RIGHT_OUTPUT_VOLUME 0x03u
#define WM8960_REG_CLOCKING1           0x04u
#define WM8960_REG_CONTROL1            0x05u
#define WM8960_REG_CONTROL2            0x06u
#define WM8960_REG_AUDIO_INTERFACE0    0x07u
#define WM8960_REG_CLOCKING2           0x08u
#define WM8960_REG_AUDIO_INTERFACE2    0x09u
#define WM8960_REG_LEFT_DAC_VOLUME     0x0au
#define WM8960_REG_RIGHT_DAC_VOLUME    0x0bu
#define WM8960_REG_RESET               0x0fu
#define WM8960_REG_LEFT_ADC_VOLUME     0x15u
#define WM8960_REG_RIGHT_ADC_VOLUME    0x16u
#define WM8960_REG_POWER_MGMT1         0x19u
#define WM8960_REG_POWER_MGMT2         0x1au
#define WM8960_REG_LEFT_INPUT_PATH     0x20u
#define WM8960_REG_RIGHT_INPUT_PATH    0x21u
#define WM8960_REG_LEFT_OUTPUT_MIX     0x22u
#define WM8960_REG_RIGHT_OUTPUT_MIX    0x25u
#define WM8960_REG_POWER_MGMT3         0x2fu

#define WM8960_CONTROL1_DAC_MUTE       0x008u

static HAL_StatusTypeDef Wm8960_WriteChecked(Wm8960 *codec, uint8_t reg,
                                              uint16_t value)
{
    return Wm8960_WriteRegister(codec, reg, value);
}

HAL_StatusTypeDef Wm8960_WriteRegister(Wm8960 *codec, uint8_t reg,
                                       uint16_t value9)
{
    uint8_t payload[2];

    if ((codec == NULL) || (codec->i2c == NULL) || (reg > 0x7fu) ||
        (value9 > 0x01ffu)) {
        return HAL_ERROR;
    }

    payload[0] = (uint8_t)((reg << 1) | ((value9 >> 8) & 0x01u));
    payload[1] = (uint8_t)(value9 & 0x00ffu);
    return HAL_I2C_Master_Transmit(codec->i2c, codec->address_8bit,
                                   payload, 2u, codec->timeout_ms);
}

HAL_StatusTypeDef Wm8960_Init(Wm8960 *codec, I2C_HandleTypeDef *i2c,
                              uint8_t address_7bit, uint32_t timeout_ms)
{
    HAL_StatusTypeDef status;

    if ((codec == NULL) || (i2c == NULL) || (address_7bit > 0x7fu) ||
        (timeout_ms == 0u)) {
        return HAL_ERROR;
    }

    codec->i2c = i2c;
    codec->address_8bit = (uint16_t)address_7bit << 1;
    codec->timeout_ms = timeout_ms;
    codec->control1_shadow = WM8960_CONTROL1_DAC_MUTE;
    codec->initialized = false;

    status = Wm8960_WriteChecked(codec, WM8960_REG_RESET, 0x000u);
    if (status != HAL_OK) {
        return status;
    }
    HAL_Delay(10u);

    /* Keep the DAC muted while VMID, converters, mixers and outputs settle. */
    status = Wm8960_WriteChecked(codec, WM8960_REG_CONTROL1,
                                 codec->control1_shadow);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_CONTROL2, 0x008u);
    if (status != HAL_OK) {
        return status;
    }

    /* Fast VMID charge: VMID=5 kOhm and VREF enabled. */
    status = Wm8960_WriteChecked(codec, WM8960_REG_POWER_MGMT1, 0x1c0u);
    if (status != HAL_OK) {
        return status;
    }
    HAL_Delay(50u);

    /* MCLK=12.288 MHz directly supplies SYSCLK. The STM32 is I2S master;
     * WM8960 is slave, 24-bit I2S, 48 kHz stereo. */
    status = Wm8960_WriteChecked(codec, WM8960_REG_CLOCKING1, 0x000u);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_CLOCKING2, 0x000u);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_AUDIO_INTERFACE0, 0x00au);
    if (status != HAL_OK) {
        return status;
    }
    /* ALRCGPIO=1: the single DACLRC/PE4 wire is also the ADC frame clock.
     * Without this bit WM8960 expects a separate ADCLRC pin and the H743's
     * internally-synchronous RX would never see valid frames. */
    status = Wm8960_WriteChecked(codec, WM8960_REG_AUDIO_INTERFACE2, 0x040u);
    if (status != HAL_OK) {
        return status;
    }

    /* Enable VREF/VMID, stereo INPUT1 PGAs, ADCs, DACs, headphone drivers
     * and both mixer paths. Operational VMID is 50 kOhm. */
    status = Wm8960_WriteChecked(codec, WM8960_REG_POWER_MGMT1, 0x0fcu);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_POWER_MGMT2, 0x1e0u);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_POWER_MGMT3, 0x03cu);
    if (status != HAL_OK) {
        return status;
    }

    status = Wm8960_WriteChecked(codec, WM8960_REG_LEFT_INPUT_PATH, 0x108u);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_RIGHT_INPUT_PATH, 0x108u);
    if (status != HAL_OK) {
        return status;
    }

    /* INPUT1 PGA, ADC, DAC and headphone output all start at 0 dB. The
     * right-channel update writes latch each stereo pair atomically. */
    status = Wm8960_WriteChecked(codec, WM8960_REG_LEFT_INPUT_VOLUME, 0x017u);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_RIGHT_INPUT_VOLUME, 0x117u);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_LEFT_ADC_VOLUME, 0x0c3u);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_RIGHT_ADC_VOLUME, 0x1c3u);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_LEFT_DAC_VOLUME, 0x0ffu);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_RIGHT_DAC_VOLUME, 0x1ffu);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_LEFT_OUTPUT_MIX, 0x100u);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_RIGHT_OUTPUT_MIX, 0x100u);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_LEFT_OUTPUT_VOLUME, 0x079u);
    if (status != HAL_OK) {
        return status;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_RIGHT_OUTPUT_VOLUME, 0x179u);
    if (status != HAL_OK) {
        return status;
    }

    codec->initialized = true;
    return HAL_OK;
}

HAL_StatusTypeDef Wm8960_SetMute(Wm8960 *codec, bool mute)
{
    uint16_t next;
    HAL_StatusTypeDef status;

    if ((codec == NULL) || !codec->initialized) {
        return HAL_ERROR;
    }

    next = codec->control1_shadow;
    if (mute) {
        next |= WM8960_CONTROL1_DAC_MUTE;
    } else {
        next &= (uint16_t)~WM8960_CONTROL1_DAC_MUTE;
    }
    status = Wm8960_WriteChecked(codec, WM8960_REG_CONTROL1, next);
    if (status == HAL_OK) {
        codec->control1_shadow = next;
    }
    return status;
}
