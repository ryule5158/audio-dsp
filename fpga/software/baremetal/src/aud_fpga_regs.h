#ifndef AUD_FPGA_REGS_H
#define AUD_FPGA_REGS_H

#include "xil_io.h"
#include "xil_types.h"

#define AUD_FPGA_REG_CONTROL        0x00U
#define AUD_FPGA_REG_GAIN_Q23       0x04U
#define AUD_FPGA_REG_DC_R_Q23       0x08U
#define AUD_FPGA_REG_PHASE_INC      0x0CU
#define AUD_FPGA_REG_OSC_SELECT     0x10U
#define AUD_FPGA_REG_SYNTH_Q23      0x14U
#define AUD_FPGA_REG_DELAY_SAMPLES  0x18U
#define AUD_FPGA_REG_FEEDBACK_Q23   0x1CU
#define AUD_FPGA_REG_DELAY_MIX_Q23  0x20U
#define AUD_FPGA_REG_ID             0x24U

#define AUD_FPGA_CONTROL_COMMIT     (1U << 0)
#define AUD_FPGA_CONTROL_ACK        (1U << 1)
#define AUD_FPGA_CONTROL_PENDING    (1U << 2)
#define AUD_FPGA_ID_AUD1            0x41554431U

#define AUD_FPGA_OSC_SAW            0U
#define AUD_FPGA_OSC_TRIANGLE       1U
#define AUD_FPGA_OSC_SQUARE         2U
#define AUD_FPGA_OSC_NOISE          3U

static inline u32 AudFpga_Read(u32 base, u32 offset)
{
    return Xil_In32(base + offset);
}

static inline void AudFpga_Write(u32 base, u32 offset, u32 value)
{
    Xil_Out32(base + offset, value);
}

static inline u32 AudFpga_PhaseIncrement(u32 frequency_hz,
                                         u32 sample_rate_hz)
{
    if ((frequency_hz == 0U) || (sample_rate_hz == 0U)) {
        return 0U;
    }
    return (u32)(((u64)frequency_hz << 32) / sample_rate_hz);
}

#endif
