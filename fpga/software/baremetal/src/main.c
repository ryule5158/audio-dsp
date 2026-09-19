#include "aud_fpga_regs.h"
#include "wm8960.h"

#include "sleep.h"
#include "xiicps.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xstatus.h"

#if defined(XPAR_AUDIO_DSP_0_S_AXI_BASEADDR)
#define AUDIO_DSP_BASEADDR XPAR_AUDIO_DSP_0_S_AXI_BASEADDR
#elif defined(XPAR_AUDIO_DSP_0_BASEADDR)
#define AUDIO_DSP_BASEADDR XPAR_AUDIO_DSP_0_BASEADDR
#else
#define AUDIO_DSP_BASEADDR 0x43C00000U
#endif

#ifndef XPAR_XIICPS_0_DEVICE_ID
#error "The soc_i2s HDF must expose PS7 I2C0."
#endif

#define WM8960_HEADPHONE_MINUS_18_DB 0x67U
#define DEMO_SAMPLE_RATE_HZ           48000U
#define DEMO_TONE_HZ                  440U
#define DEMO_SYNTH_LEVEL_Q23          0x00040000U /* 0.03125 */
#define DEMO_DELAY_SAMPLES            4800U       /* 100 ms */
#define DEMO_DELAY_FEEDBACK_Q23       0x00200000U /* 0.25 */
#define DEMO_DELAY_MIX_Q23            0x00200000U /* 0.25 */

static int InitializeIic(XIicPs *iic)
{
    XIicPs_Config *config;
    int status;

    config = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
    if (config == NULL) {
        return XST_FAILURE;
    }
    status = XIicPs_CfgInitialize(iic, config, config->BaseAddress);
    if (status != XST_SUCCESS) {
        return status;
    }
    status = XIicPs_SelfTest(iic);
    if (status != XST_SUCCESS) {
        return status;
    }
    return XIicPs_SetSClk(iic, 100000U);
}

static int ConfigureFpgaAudio(u32 base)
{
    u32 timeout;

    if (AudFpga_Read(base, AUD_FPGA_REG_ID) != AUD_FPGA_ID_AUD1) {
        return XST_FAILURE;
    }
    if ((AudFpga_Read(base, AUD_FPGA_REG_CONTROL) &
         AUD_FPGA_CONTROL_PENDING) != 0U) {
        return XST_DEVICE_BUSY;
    }

    AudFpga_Write(base, AUD_FPGA_REG_GAIN_Q23, 0x00800000U); /* 1.0 */
    AudFpga_Write(base, AUD_FPGA_REG_DC_R_Q23, 0x007F5C29U); /* 0.995 */
    AudFpga_Write(base, AUD_FPGA_REG_PHASE_INC,
                  AudFpga_PhaseIncrement(DEMO_TONE_HZ, DEMO_SAMPLE_RATE_HZ));
    AudFpga_Write(base, AUD_FPGA_REG_OSC_SELECT, AUD_FPGA_OSC_TRIANGLE);
    AudFpga_Write(base, AUD_FPGA_REG_SYNTH_Q23, DEMO_SYNTH_LEVEL_Q23);
    AudFpga_Write(base, AUD_FPGA_REG_DELAY_SAMPLES, DEMO_DELAY_SAMPLES);
    AudFpga_Write(base, AUD_FPGA_REG_FEEDBACK_Q23,
                  DEMO_DELAY_FEEDBACK_Q23);
    AudFpga_Write(base, AUD_FPGA_REG_DELAY_MIX_Q23, DEMO_DELAY_MIX_Q23);
    AudFpga_Write(base, AUD_FPGA_REG_CONTROL, AUD_FPGA_CONTROL_COMMIT);

    timeout = 1000000U;
    while (((AudFpga_Read(base, AUD_FPGA_REG_CONTROL) &
             AUD_FPGA_CONTROL_PENDING) != 0U) && (timeout != 0U)) {
        --timeout;
    }
    return (timeout == 0U) ? XST_DEVICE_BUSY : XST_SUCCESS;
}

int main(void)
{
    XIicPs iic;
    Wm8960 codec;
    int status;

    xil_printf("\r\nBX71 Zynq-7020 audio DSP / WM8960 demo\r\n");
    xil_printf("AXI audio base 0x%08lx, PS I2C0 at 100 kHz\r\n",
               (unsigned long)AUDIO_DSP_BASEADDR);

    status = InitializeIic(&iic);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: PS I2C0 init failed: %d\r\n", status);
        return status;
    }

    status = Wm8960_Initialize(&codec, &iic);
    if (status == XST_SUCCESS) {
        status = Wm8960_Configure48k24bitMaster(
            &codec, WM8960_HEADPHONE_MINUS_18_DB);
    }
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: WM8960 I2C write R%u failed: %d\r\n",
                   codec.failed_register, status);
        return status;
    }
    xil_printf("WM8960: 24 MHz MCLK, PLL, 48 kHz, 24-bit I2S master OK\r\n");

    status = ConfigureFpgaAudio(AUDIO_DSP_BASEADDR);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: FPGA register/commit failed: %d ID=0x%08lx\r\n",
                   status,
                   (unsigned long)AudFpga_Read(AUDIO_DSP_BASEADDR,
                                               AUD_FPGA_REG_ID));
        return status;
    }

    xil_printf("FPGA: ID=AUD1, 440 Hz low-level triangle + 100 ms delay active\r\n");
    xil_printf("AUDIO_DSP_READY\r\n");

    for (;;) {
        sleep(1U);
    }
}
