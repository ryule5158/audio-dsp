#ifndef LVGL_BOARD_CONFIG_H
#define LVGL_BOARD_CONFIG_H

/* REFERENCE ONLY: verify the panel and carrier wiring before flashing.
 * This independent target owns SPI1 and PC4..PC7; it does not enable audio.
 * BL is a logic-level enable for an external current-limited backlight driver,
 * never a connection from a GPIO directly to a bare backlight LED. */
#define LVGL_ST7789_WIDTH             240u
#define LVGL_ST7789_HEIGHT            240u
#define LVGL_ST7789_BUFFER_LINES      24u
#define LVGL_ST7789_MAX_TRANSFER      512u
#define LVGL_ST7789_SPI_TIMEOUT_MS    100u
#define LVGL_ST7789_SPI_DIVIDER       64u
#define LVGL_ST7789_SPI_PRESCALER     SPI_BAUDRATEPRESCALER_64
#define LVGL_ST7789_SPI_MAX_HZ        10000000u
#define LVGL_ST7789_X_GAP             0u
#define LVGL_ST7789_Y_GAP             0u
#define LVGL_ST7789_FLAGS             LV_LCD_FLAG_NONE
#define LVGL_ST7789_INVERT            false

/* TX-only SPI1: PA5=SCK and PA7=MOSI (AF5); PA6 is left untouched. */
#define LVGL_ST7789_CS_PORT           GPIOC
#define LVGL_ST7789_CS_PIN            GPIO_PIN_4
#define LVGL_ST7789_DC_PORT           GPIOC
#define LVGL_ST7789_DC_PIN            GPIO_PIN_5
#define LVGL_ST7789_RST_PORT          GPIOC
#define LVGL_ST7789_RST_PIN           GPIO_PIN_6
#define LVGL_ST7789_BL_PORT           GPIOC
#define LVGL_ST7789_BL_PIN            GPIO_PIN_7

#if LVGL_ST7789_MAX_TRANSFER == 0 || (LVGL_ST7789_MAX_TRANSFER & 1u) != 0
#error "RGB565 transfer size must be a nonzero even number"
#endif
#if LVGL_ST7789_MAX_TRANSFER > 65535u
#error "HAL SPI transfer length is uint16_t"
#endif

#endif
