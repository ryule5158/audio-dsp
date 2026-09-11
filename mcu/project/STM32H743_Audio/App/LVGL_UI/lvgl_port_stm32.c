/**
 * @file lvgl_port_stm32.c
 * @brief Synchronous SPI ST7789 port following the LVGL 9 display API.
 *
 * The callbacks match lv_st7789_create() from the vendored LVGL v9.5.0
 * source.  Transfers are polling transfers first; this makes the template
 * deterministic and easy to replace with DMA after the board wiring is
 * measured. The callback releases LVGL's buffer only after synchronous
 * completion or a latched transfer abort, distinguished by diagnostics.
 */

#include "lvgl_port_stm32.h"

#include "main.h"
#include "stm32h7xx_hal_spi.h"
#include "lvgl_board_config.h"

#include <stddef.h>

static SPI_HandleTypeDef s_lvgl_spi;
static lv_display_t * s_lvgl_display;
static bool s_started;
static lv_point_t s_pointer;

volatile int32_t g_lvgl_port_status = LVGL_PORT_NOT_INITIALIZED;
volatile uint32_t g_lvgl_spi_clock_hz;
volatile uint32_t g_lvgl_spi_error_code;
volatile uint32_t g_lvgl_transfer_errors;
volatile uint32_t g_lvgl_flush_completed;
volatile uint32_t g_lvgl_flush_aborted;

/* Two partial draw buffers permit LVGL to render while the previous buffer
 * is being consumed by a future DMA implementation.  The current polling
 * implementation still uses both buffers and remains fully synchronous. */
static uint8_t s_draw_buf_a[LVGL_ST7789_WIDTH * LVGL_ST7789_BUFFER_LINES * 2u]
    __attribute__((aligned(32)));
static uint8_t s_draw_buf_b[LVGL_ST7789_WIDTH * LVGL_ST7789_BUFFER_LINES * 2u]
    __attribute__((aligned(32)));
static uint8_t s_swap_buf[LVGL_ST7789_MAX_TRANSFER]
    __attribute__((aligned(4)));

static void LvglPort_Select(void)
{
    HAL_GPIO_WritePin(LVGL_ST7789_CS_PORT, LVGL_ST7789_CS_PIN, GPIO_PIN_RESET);
}

static void LvglPort_Unselect(void)
{
    HAL_GPIO_WritePin(LVGL_ST7789_CS_PORT, LVGL_ST7789_CS_PIN, GPIO_PIN_SET);
}

static void LvglPort_Fault(LvglPortStatus status)
{
    if (g_lvgl_port_status >= 0) {
        g_lvgl_port_status = (int32_t)status;
        ++g_lvgl_transfer_errors;
    }
    HAL_GPIO_WritePin(LVGL_ST7789_BL_PORT, LVGL_ST7789_BL_PIN, GPIO_PIN_RESET);
}

static int LvglPort_Write(const uint8_t * data, size_t size, GPIO_PinState dc)
{
    size_t offset = 0u;

    if (g_lvgl_port_status < 0) {
        return -1;
    }
    if ((data == NULL) && (size != 0u)) {
        LvglPort_Fault(LVGL_PORT_BAD_TRANSFER);
        return -1;
    }
    HAL_GPIO_WritePin(LVGL_ST7789_DC_PORT, LVGL_ST7789_DC_PIN, dc);
    while (offset < size) {
        size_t chunk = size - offset;
        if (chunk > LVGL_ST7789_MAX_TRANSFER) {
            chunk = LVGL_ST7789_MAX_TRANSFER;
        }
        if (HAL_SPI_Transmit(&s_lvgl_spi, (uint8_t *)&data[offset],
                (uint16_t)chunk, LVGL_ST7789_SPI_TIMEOUT_MS) != HAL_OK) {
            g_lvgl_spi_error_code = HAL_SPI_GetError(&s_lvgl_spi);
            LvglPort_Fault(LVGL_PORT_SPI_TRANSFER_FAILED);
            return -1;
        }
        offset += chunk;
    }
    return 0;
}

static void LvglPort_SendCommand(lv_display_t * display, const uint8_t * command,
                                 size_t command_size, const uint8_t * param,
                                 size_t param_size)
{
    (void)display;
    if (g_lvgl_port_status < 0) {
        return;
    }
    LvglPort_Select();
    if (LvglPort_Write(command, command_size, GPIO_PIN_RESET) == 0) {
        (void)LvglPort_Write(param, param_size, GPIO_PIN_SET);
    }
    LvglPort_Unselect();
}

static void LvglPort_SendColor(lv_display_t * display, const uint8_t * command,
                               size_t command_size, uint8_t * param,
                               size_t param_size)
{
    size_t offset = 0u;

    if ((param == NULL && param_size != 0u) || (param_size & 1u) != 0u) {
        LvglPort_Fault(LVGL_PORT_BAD_TRANSFER);
    }
    if (g_lvgl_port_status < 0) {
        ++g_lvgl_flush_aborted;
        lv_display_flush_ready(display);
        return;
    }
    LvglPort_Select();
    (void)LvglPort_Write(command, command_size, GPIO_PIN_RESET);

    /* LVGL stores RGB565 in the MCU's native little-endian order while the
     * ST7789 serial bus expects the high byte first.  Swap in bounded chunks
     * so the driver never writes beyond its small transfer scratch buffer. */
    while ((offset < param_size) && (g_lvgl_port_status >= 0)) {
        size_t chunk = param_size - offset;
        size_t i;
        if (chunk > sizeof(s_swap_buf)) {
            chunk = sizeof(s_swap_buf);
        }
        for (i = 0u; i < chunk; i += 2u) {
            s_swap_buf[i] = param[offset + i + 1u];
            s_swap_buf[i + 1u] = param[offset + i];
        }
        if (LvglPort_Write(s_swap_buf, chunk, GPIO_PIN_SET) == 0) {
            offset += chunk;
        }
    }

    LvglPort_Unselect();
    if (g_lvgl_port_status >= 0) {
        ++g_lvgl_flush_completed;
        HAL_GPIO_WritePin(LVGL_ST7789_BL_PORT, LVGL_ST7789_BL_PIN, GPIO_PIN_SET);
    }
    else {
        ++g_lvgl_flush_aborted;
    }
    /* Release buffer ownership on both completion and a latched abort.
     * This is not a success indication for the panel; counters distinguish it. */
    lv_display_flush_ready(display);
}

void HAL_SPI_MspInit(SPI_HandleTypeDef * hspi)
{
    GPIO_InitTypeDef gpio = {0};

    if ((hspi == NULL) || (hspi->Instance != SPI1)) {
        return;
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();

    gpio.Pin = GPIO_PIN_5 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &gpio);
}

static int LvglPort_SpiInit(void)
{
    HAL_StatusTypeDef status;
    RCC_PeriphCLKInitTypeDef clock = {0};

    /* Select the existing PLL1 Q output; do not retune PLL1/PLL3 or SAI. */
    clock.PeriphClockSelection = RCC_PERIPHCLK_SPI1;
    clock.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&clock) != HAL_OK) {
        LvglPort_Fault(LVGL_PORT_CLOCK_FAILED);
        return -1;
    }
    g_lvgl_spi_clock_hz = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SPI1) /
        LVGL_ST7789_SPI_DIVIDER;
    if (g_lvgl_spi_clock_hz == 0u ||
        g_lvgl_spi_clock_hz > LVGL_ST7789_SPI_MAX_HZ) {
        LvglPort_Fault(LVGL_PORT_CLOCK_FAILED);
        return -1;
    }

    s_lvgl_spi.Instance = SPI1;
    s_lvgl_spi.Init.Mode = SPI_MODE_MASTER;
    s_lvgl_spi.Init.Direction = SPI_DIRECTION_2LINES_TXONLY;
    s_lvgl_spi.Init.DataSize = SPI_DATASIZE_8BIT;
    s_lvgl_spi.Init.CLKPolarity = SPI_POLARITY_HIGH;
    s_lvgl_spi.Init.CLKPhase = SPI_PHASE_2EDGE;
    s_lvgl_spi.Init.NSS = SPI_NSS_SOFT;
    s_lvgl_spi.Init.BaudRatePrescaler = LVGL_ST7789_SPI_PRESCALER;
    s_lvgl_spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    s_lvgl_spi.Init.TIMode = SPI_TIMODE_DISABLE;
    s_lvgl_spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    s_lvgl_spi.Init.CRCPolynomial = 0x7u;
    s_lvgl_spi.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    s_lvgl_spi.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    s_lvgl_spi.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    s_lvgl_spi.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    s_lvgl_spi.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    s_lvgl_spi.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    s_lvgl_spi.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    s_lvgl_spi.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    s_lvgl_spi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    s_lvgl_spi.Init.IOSwap = SPI_IO_SWAP_DISABLE;

    status = HAL_SPI_Init(&s_lvgl_spi);
    if (status != HAL_OK) {
        g_lvgl_spi_error_code = HAL_SPI_GetError(&s_lvgl_spi);
        LvglPort_Fault(LVGL_PORT_SPI_INIT_FAILED);
    }
    return (status == HAL_OK) ? 0 : -1;
}

static void LvglPort_GpioInit(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(LVGL_ST7789_CS_PORT, LVGL_ST7789_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LVGL_ST7789_DC_PORT, LVGL_ST7789_DC_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LVGL_ST7789_RST_PORT, LVGL_ST7789_RST_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LVGL_ST7789_BL_PORT, LVGL_ST7789_BL_PIN, GPIO_PIN_RESET);

    gpio.Pin = LVGL_ST7789_CS_PIN | LVGL_ST7789_DC_PIN |
               LVGL_ST7789_RST_PIN | LVGL_ST7789_BL_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOC, &gpio);
}

static void LvglPort_ResetPanel(void)
{
    HAL_GPIO_WritePin(LVGL_ST7789_RST_PORT, LVGL_ST7789_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10u);
    HAL_GPIO_WritePin(LVGL_ST7789_RST_PORT, LVGL_ST7789_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(120u);
}

__weak int LvglPort_ReadPointer(int32_t * x, int32_t * y, bool * pressed)
{
    (void)x;
    (void)y;
    (void)pressed;
    return 0;
}

static void LvglPort_PointerRead(lv_indev_t * indev, lv_indev_data_t * data)
{
    int32_t x = s_pointer.x;
    int32_t y = s_pointer.y;
    bool pressed = false;
    (void)indev;

    if (LvglPort_ReadPointer(&x, &y, &pressed) == 1) {
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x >= (int32_t)LVGL_ST7789_WIDTH) x = (int32_t)LVGL_ST7789_WIDTH - 1;
        if (y >= (int32_t)LVGL_ST7789_HEIGHT) y = (int32_t)LVGL_ST7789_HEIGHT - 1;
        s_pointer.x = x;
        s_pointer.y = y;
    }
    else {
        pressed = false;
    }
    data->point = s_pointer;
    data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

int LvglPort_Init(void)
{
    lv_indev_t * pointer;
    if (s_started) {
        return (int)g_lvgl_port_status;
    }
    s_started = true;
    g_lvgl_port_status = LVGL_PORT_INITIALIZING;
    lv_init();
    /* lv_init clears global state, so callbacks must be registered AFTER it. */
    lv_tick_set_cb(HAL_GetTick);
    lv_delay_set_cb(HAL_Delay);
    LvglPort_GpioInit();
    if (LvglPort_SpiInit() != 0) {
        return (int)g_lvgl_port_status;
    }

    LvglPort_ResetPanel();

    s_lvgl_display = lv_st7789_create(LVGL_ST7789_WIDTH, LVGL_ST7789_HEIGHT,
                                      LVGL_ST7789_FLAGS, LvglPort_SendCommand,
                                      LvglPort_SendColor);
    if (s_lvgl_display == NULL) {
        LvglPort_Fault(LVGL_PORT_ALLOCATION_FAILED);
        return (int)g_lvgl_port_status;
    }

    lv_st7789_set_gap(s_lvgl_display, LVGL_ST7789_X_GAP, LVGL_ST7789_Y_GAP);
    lv_st7789_set_invert(s_lvgl_display, LVGL_ST7789_INVERT);
    if (g_lvgl_port_status < 0) {
        lv_display_delete(s_lvgl_display);
        s_lvgl_display = NULL;
        return (int)g_lvgl_port_status;
    }
    lv_display_set_color_format(s_lvgl_display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(s_lvgl_display, s_draw_buf_a, s_draw_buf_b,
                           sizeof(s_draw_buf_a), LV_DISPLAY_RENDER_MODE_PARTIAL);
    pointer = lv_indev_create();
    if (pointer == NULL) {
        LvglPort_Fault(LVGL_PORT_ALLOCATION_FAILED);
        lv_display_delete(s_lvgl_display);
        s_lvgl_display = NULL;
        return (int)g_lvgl_port_status;
    }
    lv_indev_set_type(pointer, LV_INDEV_TYPE_POINTER);
    lv_indev_set_display(pointer, s_lvgl_display);
    lv_indev_set_read_cb(pointer, LvglPort_PointerRead);
    g_lvgl_port_status = LVGL_PORT_READY;
    return 0;
}

lv_display_t * LvglPort_Display(void)
{
    return s_lvgl_display;
}
