# STM32H743 manual LVGL UI port

This directory is an optional, project-local port of **LVGL v9.5.0** for the
`STM32H743_Audio_LVGL_UI` Keil target.  It is intentionally separate from
CubeMX-generated code and from the audio real-time callback.

## Source and API contract

- Upstream tag: `v9.5.0`
- Upstream commit: `85aa60d18b3d5e5588d7b247abf90198f07c8a63`
- Source location: `App/LVGL/src` (official source tree, copied without the
  examples/demos/test trees)
- Configuration: `App/LVGL/lv_conf.h`, derived from the official
  `lv_conf_template.h`; no Keil Pack or RTE component is used.
- Port sequence follows the official LVGL display integration contract:
  `lv_init` → tick callback → display creation → draw buffers → flush callback
  → periodic `lv_timer_handler`.

## Reference wiring

| Function | STM32H743 pin |
| --- | --- |
| SPI1 SCK/MOSI (TX-only) | PA5 / PA7 (AF5); PA6 untouched |
| ST7789 CS/DC/RESET/BL | PC4 / PC5 / PC6 / PC7 |

The pins are a reference contract for a 240×240 RGB565 ST7789 SPI panel.  The
panel's voltage, level shifting, reset timing, backlight current limit and
grounding must be checked against the actual hardware before energizing it.

All reference settings are explicit in `lvgl_board_config.h`; this is not an
identification of the user's actual carrier or screen. SPI mode 3 uses the
existing PLL1 Q output divided by 64 (6.25 MHz with this project's 400 MHz
PLL1 Q); initialization rejects zero or greater-than-10-MHz computed SCK.
The actual waveform must still be measured. Panel-specific offsets, inversion,
RGB/BGR ordering and gamma/power settings must match the exact ST7789 module.
The stock LVGL ST7789 initialization is used, not a verified panel-specific tune.
PC7 controls an external current-limited backlight driver's logic enable, never
a bare LED load. It stays off until the first successful SPI flush.

## Runtime and fault contract

Initialization order is `lv_init()` -> `lv_tick_set_cb(HAL_GetTick)` ->
`lv_delay_set_cb(HAL_Delay)` -> GPIO/SPI -> ST7789 -> partial buffers -> pointer
input -> UI. The existing SysTick remains the sole timebase; do not also call
`lv_tick_inc`. Foreground `LvglUi_Service()` runs the LVGL timer handler; no
LVGL function or polling SPI transfer runs from an audio/DMA interrupt.

Two 240 x 24 x 2-byte buffers use 23,040 bytes; RGB565 byte swapping uses a
512-byte scratch buffer. Transfers are synchronous, bounded to 100 ms per
chunk; there is no display DMA or cache-coherency claim. On the first HAL
failure the port latches a fault, disables backlight and stops future rendering.
It releases any outstanding LVGL buffer on abort as well as success, without
counting the abort as a completed flush. Recovery requires reset; this template
does not retry an unknown panel indefinitely.

Debugger values:

- `g_lvgl_port_status` / `g_lvgl_ui_status`: 0 ready, 1 uninitialized,
  2 initializing; -1 clock, -2 SPI init, -3 SPI transfer, -4 invalid RGB565
  transfer, -5 allocation failure.
- `g_lvgl_spi_clock_hz`, `g_lvgl_spi_error_code`, `g_lvgl_transfer_errors`.
- `g_lvgl_flush_completed` and `g_lvgl_flush_aborted`.

A ready state and successful SPI writes only describe the MCU transport.
There is no MISO/readback/ACK and therefore no disconnected-panel detection.
The fixed LVGL heap uses the upstream allocation/assert behavior; arbitrary
user-created widgets can exhaust it and require a separately sized memory budget.

## Input and UI extension

Override the weak `LvglPort_ReadPointer` hook with a foreground board-specific
touch reader. Return 1 and display-space x/y/pressed for a valid sample, or 0
when unavailable. The built-in default returns 0, remains released and touches
no bus; no physical touch controller has been selected or verified. Interrupts
may queue input data, but must not call LVGL. Pointer samples are clamped to the
display bounds. Slider events update `g_lvgl_ui_cutoff_hz` and
`g_lvgl_ui_mix_percent`; these are UI-only values, not live audio controls.
Connect them through an application parameter mailbox only in a separately
verified audio+UI profile.

## Isolation

Only the optional LVGL target includes `App/LVGL_UI`, the LVGL source group,
the target-local SPI HAL source and the `HAL_SPI_MODULE_ENABLED` define.  The
SelfTest, WM8960 stream and Generic DSP targets do not compile or link any
LVGL object and keep their CubeMX audio path unchanged.

All four targets contain identical Keil project-item lists, with explicit
per-file `IncludeInBuild` values. This is required by uVision's shared project
model: simply omitting a group in the other targets does not reliably exclude
its files. The LVGL target still contains the 49 permissive DSP sources and
startup self-test, but it does not start WM8960 or SAI DMA. Its SPI setup lives
outside generated code so the common audio `.ioc` is not changed.

CubeMX removes unused peripheral driver files during regeneration. The official
regeneration script restores only SPI C/header dependencies from the pinned
H7 V1.13.0 package, verifies their SHA-256 values, and refuses to overwrite a
modified local dependency. Regenerate through `mcu/scripts/verify_all.ps1` to
restore the four target definitions and run all isolation/build gates.

Official references: [STM32 HAL integration](https://lvgl.io/docs/open/9.5/integration/chip_vendors/stm32/add_lvgl_to_your_stm32_project.html),
[ST7789 driver](https://lvgl.io/docs/open/9.5/integration/external_display_controllers/st7789.html).
