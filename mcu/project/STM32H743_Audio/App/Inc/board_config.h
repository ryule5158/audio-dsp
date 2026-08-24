#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/*
 * Reference wiring contract for this template (not a claim about an existing
 * user PCB): STM32H743IIT6 LQFP176 + external WM8960 module.
 *
 * SAI1 block A is the 48 kHz I2S master transmitter:
 *   PE6  -> WM8960 DACDAT
 *   PG7  -> WM8960 MCLK
 *   PE5  -> WM8960 BCLK
 *   PE4  -> WM8960 LRCLK
 * SAI1 block B is an internally synchronous receiver:
 *   PE3  <- WM8960 ADCDAT
 * I2C1 control bus:
 *   PB8  -> WM8960 SCL, external pull-up required
 *   PB9  <-> WM8960 SDA, external pull-up required
 * Common 3.3 V logic supply and ground are required.
 */

#define AUDIO_BOARD_SAMPLE_RATE_HZ       48000u
#define AUDIO_BOARD_CHANNELS             2u
#define AUDIO_BOARD_BITS_PER_SAMPLE      24u
#define AUDIO_BOARD_DMA_HALF_FRAMES      64u
#define AUDIO_BOARD_DELAY_TIME_MS         100u
#define AUDIO_BOARD_WM8960_ADDR_7BIT      0x1au
#define AUDIO_BOARD_WM8960_I2C_TIMEOUT_MS 100u

/*
 * Zero is the safe repository default: firmware runs deterministic DSP
 * self-test and does not touch the external codec or start SAI DMA. Set to 1
 * only after the wiring contract above has been checked on the target PCB.
 */
#ifndef AUDIO_BOARD_ENABLE_WM8960_STREAM
#define AUDIO_BOARD_ENABLE_WM8960_STREAM  0u
#endif

/* DMA1/2 can access D2 SRAM at 0x30000000. The linker and MPU reserve the
 * first 32 KiB as a non-cacheable .audio_dma region. */
#define AUDIO_BOARD_DMA_REGION_BASE       0x30000000u
#define AUDIO_BOARD_DMA_REGION_BYTES      0x00008000u

#endif
