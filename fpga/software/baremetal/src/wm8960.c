#include "wm8960.h"

#include "sleep.h"
#include "xstatus.h"

enum {
    WM8960_REG_LEFT_INPUT_VOLUME = 0x00,
    WM8960_REG_RIGHT_INPUT_VOLUME = 0x01,
    WM8960_REG_LEFT_HEADPHONE_VOLUME = 0x02,
    WM8960_REG_RIGHT_HEADPHONE_VOLUME = 0x03,
    WM8960_REG_CLOCKING_1 = 0x04,
    WM8960_REG_ADC_DAC_CONTROL_1 = 0x05,
    WM8960_REG_AUDIO_INTERFACE_1 = 0x07,
    WM8960_REG_CLOCKING_2 = 0x08,
    WM8960_REG_AUDIO_INTERFACE_2 = 0x09,
    WM8960_REG_LEFT_DAC_VOLUME = 0x0A,
    WM8960_REG_RIGHT_DAC_VOLUME = 0x0B,
    WM8960_REG_RESET = 0x0F,
    WM8960_REG_LEFT_ADC_VOLUME = 0x15,
    WM8960_REG_RIGHT_ADC_VOLUME = 0x16,
    WM8960_REG_POWER_1 = 0x19,
    WM8960_REG_POWER_2 = 0x1A,
    WM8960_REG_ANTI_POP_1 = 0x1C,
    WM8960_REG_LEFT_INPUT_PATH = 0x20,
    WM8960_REG_RIGHT_INPUT_PATH = 0x21,
    WM8960_REG_LEFT_OUTPUT_MIXER = 0x22,
    WM8960_REG_RIGHT_OUTPUT_MIXER = 0x25,
    WM8960_REG_POWER_3 = 0x2F,
    WM8960_REG_PLL_N = 0x34,
    WM8960_REG_PLL_K1 = 0x35,
    WM8960_REG_PLL_K2 = 0x36,
    WM8960_REG_PLL_K3 = 0x37
};

typedef struct {
    u8 reg;
    u16 value;
} Wm8960RegisterValue;

static int Wm8960_WaitForIdle(XIicPs *iic)
{
    u32 timeout = 100000U;

    while ((XIicPs_BusIsBusy(iic) != 0) && (timeout != 0U)) {
        --timeout;
    }
    return (timeout == 0U) ? XST_DEVICE_BUSY : XST_SUCCESS;
}

int Wm8960_WriteRegister(Wm8960 *codec, u8 reg, u16 value)
{
    u8 message[2];
    int status;

    if ((codec == NULL) || (codec->iic == NULL) || (reg > 0x7FU) ||
        (value > 0x01FFU)) {
        return XST_INVALID_PARAM;
    }

    status = Wm8960_WaitForIdle(codec->iic);
    if (status != XST_SUCCESS) {
        codec->failed_register = reg;
        return status;
    }

    /* WM8960 two-wire control is a seven-bit register plus a nine-bit value. */
    message[0] = (u8)((reg << 1) | ((value >> 8) & 0x01U));
    message[1] = (u8)(value & 0x00FFU);
    status = XIicPs_MasterSendPolled(codec->iic, message, 2,
                                     WM8960_I2C_ADDRESS);
    if (status == XST_SUCCESS) {
        status = Wm8960_WaitForIdle(codec->iic);
    }
    if (status != XST_SUCCESS) {
        codec->failed_register = reg;
    }
    return status;
}

static int Wm8960_WriteSequence(Wm8960 *codec,
                                const Wm8960RegisterValue *sequence,
                                u32 count)
{
    u32 index;
    int status;

    for (index = 0U; index < count; ++index) {
        status = Wm8960_WriteRegister(codec, sequence[index].reg,
                                      sequence[index].value);
        if (status != XST_SUCCESS) {
            return status;
        }
    }
    return XST_SUCCESS;
}

int Wm8960_Initialize(Wm8960 *codec, XIicPs *iic)
{
    if ((codec == NULL) || (iic == NULL)) {
        return XST_INVALID_PARAM;
    }
    codec->iic = iic;
    codec->failed_register = 0xFFU;
    return Wm8960_WriteRegister(codec, WM8960_REG_RESET, 0U);
}

int Wm8960_Configure48k24bitMaster(Wm8960 *codec,
                                  u8 headphone_volume)
{
    int status;

    /* Values below implement the WM8960 datasheet's 24 MHz -> 48 kHz
     * fractional-PLL case. PLL output is 24.576 MHz and SYSCLKDIV=2 gives
     * 12.288 MHz; BCLKDIV=4 then gives 3.072 MHz (64 clocks per frame).
     * The Class-D speaker path is deliberately left powered down. */
    static const Wm8960RegisterValue before_pll[] = {
        {WM8960_REG_ADC_DAC_CONTROL_1, 0x008U}, /* DAC soft mute */
        {WM8960_REG_ANTI_POP_1, 0x09CU},
        {WM8960_REG_POWER_1, 0x080U},           /* VMID 50k ramp */
        {WM8960_REG_CLOCKING_1, 0x004U},        /* SYSCLKDIV=2, MCLK */
        {WM8960_REG_CLOCKING_2, 0x004U},        /* BCLKDIV=4 */
        {WM8960_REG_AUDIO_INTERFACE_1, 0x04AU}, /* master, I2S, 24-bit */
        /* The Waveshare header exposes one LRCLK.  ALRCGPIO makes DACLRC the
         * common ADC/DAC frame clock, as required by the WM8960 data sheet. */
        {WM8960_REG_AUDIO_INTERFACE_2, 0x040U},
        {WM8960_REG_PLL_N, 0x038U},             /* prescale, frac, N=8 */
        {WM8960_REG_PLL_K1, 0x031U},
        {WM8960_REG_PLL_K2, 0x026U},
        {WM8960_REG_PLL_K3, 0x0E9U}
    };
    static const Wm8960RegisterValue signal_path[] = {
        {WM8960_REG_POWER_1, 0x0FEU}, /* VREF, mic bias, input PGAs, ADCs */
        {WM8960_REG_POWER_2, 0x1E1U}, /* DACs, headphone buffers, PLL */
        {WM8960_REG_POWER_3, 0x03CU}, /* input and output mixers */
        {WM8960_REG_LEFT_INPUT_PATH, 0x108U},
        {WM8960_REG_RIGHT_INPUT_PATH, 0x108U},
        {WM8960_REG_LEFT_INPUT_VOLUME, 0x017U},
        {WM8960_REG_RIGHT_INPUT_VOLUME, 0x117U},
        {WM8960_REG_LEFT_ADC_VOLUME, 0x0C3U},
        {WM8960_REG_RIGHT_ADC_VOLUME, 0x1C3U},
        {WM8960_REG_LEFT_DAC_VOLUME, 0x0FFU},
        {WM8960_REG_RIGHT_DAC_VOLUME, 0x1FFU},
        {WM8960_REG_LEFT_OUTPUT_MIXER, 0x100U},
        {WM8960_REG_RIGHT_OUTPUT_MIXER, 0x100U}
    };

    if ((codec == NULL) || (headphone_volume > 0x7FU)) {
        return XST_INVALID_PARAM;
    }

    status = Wm8960_WriteSequence(codec, before_pll,
                                  sizeof(before_pll) / sizeof(before_pll[0]));
    if (status != XST_SUCCESS) {
        return status;
    }

    usleep(100000U);
    status = Wm8960_WriteSequence(codec, signal_path,
                                  sizeof(signal_path) / sizeof(signal_path[0]));
    if (status != XST_SUCCESS) {
        return status;
    }

    /* Fractional PLL lock time. Keep CLKSEL on MCLK until this delay expires. */
    usleep(250000U);
    status = Wm8960_WriteRegister(codec, WM8960_REG_CLOCKING_1, 0x005U);
    if (status != XST_SUCCESS) {
        return status;
    }

    status = Wm8960_WriteRegister(codec, WM8960_REG_LEFT_HEADPHONE_VOLUME,
                                  (u16)headphone_volume);
    if (status != XST_SUCCESS) {
        return status;
    }
    status = Wm8960_WriteRegister(codec, WM8960_REG_RIGHT_HEADPHONE_VOLUME,
                                  (u16)(0x100U | headphone_volume));
    if (status != XST_SUCCESS) {
        return status;
    }

    status = Wm8960_WriteRegister(codec, WM8960_REG_ANTI_POP_1, 0x008U);
    if (status != XST_SUCCESS) {
        return status;
    }
    return Wm8960_WriteRegister(codec, WM8960_REG_ADC_DAC_CONTROL_1, 0x000U);
}
