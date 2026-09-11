/**
 * @file lvgl_ui.h
 * @brief Optional LVGL user interface entry points.
 */

#ifndef LVGL_UI_H
#define LVGL_UI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* UI-only values; not connected to the real-time audio graph. */
extern volatile int32_t g_lvgl_ui_status;
extern volatile int32_t g_lvgl_ui_cutoff_hz;
extern volatile int32_t g_lvgl_ui_mix_percent;

int LvglUi_Init(void);
void LvglUi_Service(void);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_UI_H */
