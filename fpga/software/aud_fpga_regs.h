#ifndef AUD_FPGA_REGS_H
#define AUD_FPGA_REGS_H

#include <stdint.h>

#define AUD_FPGA_REG_CONTROL        0x00u
#define AUD_FPGA_REG_GAIN_Q23       0x04u
#define AUD_FPGA_REG_DC_R_Q23       0x08u
#define AUD_FPGA_REG_PHASE_INC      0x0cu
#define AUD_FPGA_REG_OSC_SELECT     0x10u
#define AUD_FPGA_REG_SYNTH_Q23      0x14u
#define AUD_FPGA_REG_DELAY_SAMPLES  0x18u
#define AUD_FPGA_REG_FEEDBACK_Q23   0x1cu
#define AUD_FPGA_REG_DELAY_MIX_Q23  0x20u
#define AUD_FPGA_REG_ID             0x24u

#define AUD_FPGA_CONTROL_COMMIT     (1u << 0)
#define AUD_FPGA_CONTROL_ACK        (1u << 1)
#define AUD_FPGA_CONTROL_PENDING    (1u << 2)
#define AUD_FPGA_ID_AUD1            0x41554431u

static inline uint32_t AudFpga_PhaseIncrement(float frequency_hz,
                                               float sample_rate_hz)
{
    double increment = (double)frequency_hz * 4294967296.0 /
                       (double)sample_rate_hz;
    if (increment <= 0.0) return 0u;
    if (increment >= 4294967295.0) return UINT32_MAX;
    return (uint32_t)(increment + 0.5);
}

#endif
