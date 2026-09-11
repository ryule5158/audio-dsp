/**
 * @file lvgl_port_stm32.h
 * @brief Manual LVGL 9.5 display port for the STM32H743 reference board.
 *
 * This port is deliberately kept outside the CubeMX generated sources.  It
 * owns the SPI1/GPIO resources used by the optional LVGL_UI target and does
 * not change the audio targets.
 */

#ifndef LVGL_PORT_STM32_H
#define LVGL_PORT_STM32_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LVGL_PORT_READY = 0,
    LVGL_PORT_NOT_INITIALIZED = 1,
    LVGL_PORT_INITIALIZING = 2,
    LVGL_PORT_CLOCK_FAILED = -1,
    LVGL_PORT_SPI_INIT_FAILED = -2,
    LVGL_PORT_SPI_TRANSFER_FAILED = -3,
    LVGL_PORT_BAD_TRANSFER = -4,
    LVGL_PORT_ALLOCATION_FAILED = -5
} LvglPortStatus;

/* Debugger-readable transport diagnostics; READY is not a panel ACK. */
extern volatile int32_t g_lvgl_port_status;
extern volatile uint32_t g_lvgl_spi_clock_hz;
extern volatile uint32_t g_lvgl_spi_error_code;
extern volatile uint32_t g_lvgl_transfer_errors;
extern volatile uint32_t g_lvgl_flush_completed;
extern volatile uint32_t g_lvgl_flush_aborted;

/** Initialize once. A transport failure is latched until reset. */
int LvglPort_Init(void);

/** Return the display created by LvglPort_Init, or NULL before init. */
lv_display_t * LvglPort_Display(void);

/** Optional board hook, called only from the foreground LVGL timer handler.
 * Override the weak default with a real touch driver. Return 1 for a valid
 * sample (including release), 0 if unavailable; coordinates are display pixels.
 * The default returns 0 and accesses no I2C/GPIO/input device. Never call LVGL
 * from an ISR; a touch ISR may only queue data for this foreground hook. */
int LvglPort_ReadPointer(int32_t * x, int32_t * y, bool * pressed);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_PORT_STM32_H */
