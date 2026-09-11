/**
 * @file lvgl_ui.c
 * @brief Small synth/effect control surface used by the LVGL template.
 *
 * Widgets are intentionally independent of the audio callback.  Application
 * code can later attach slider events to an AudioDsp parameter mailbox without
 * making the display library part of the real-time SAI path.
 */

#include "lvgl_ui.h"

#include "lvgl_port_stm32.h"

volatile int32_t g_lvgl_ui_status = LVGL_PORT_NOT_INITIALIZED;
volatile int32_t g_lvgl_ui_cutoff_hz = 1200;
volatile int32_t g_lvgl_ui_mix_percent = 50;
static bool s_ui_ready;

static void LvglUi_CutoffChanged(lv_event_t * event)
{
    lv_obj_t * slider = lv_event_get_target_obj(event);
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(event);
    g_lvgl_ui_cutoff_hz = lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "CUTOFF %d Hz", (int)g_lvgl_ui_cutoff_hz);
}

static void LvglUi_MixChanged(lv_event_t * event)
{
    lv_obj_t * slider = lv_event_get_target_obj(event);
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(event);
    g_lvgl_ui_mix_percent = lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "MIX %d %%", (int)g_lvgl_ui_mix_percent);
}

int LvglUi_Init(void)
{
    lv_obj_t * screen;
    lv_obj_t * title;
    lv_obj_t * cutoff_label;
    lv_obj_t * mix_label;
    lv_obj_t * cutoff_slider;
    lv_obj_t * mix_slider;

    if (s_ui_ready) {
        return (int)g_lvgl_ui_status;
    }
    g_lvgl_ui_status = LvglPort_Init();
    if (g_lvgl_ui_status != LVGL_PORT_READY) {
        return (int)g_lvgl_ui_status;
    }

    screen = lv_screen_active();
    title = lv_label_create(screen);
    lv_label_set_text(title, "AUDIO DSP");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    cutoff_label = lv_label_create(screen);
    lv_label_set_text(cutoff_label, "CUTOFF 1200 Hz");
    lv_obj_align(cutoff_label, LV_ALIGN_TOP_LEFT, 16, 52);

    cutoff_slider = lv_slider_create(screen);
    lv_slider_set_range(cutoff_slider, 20, 20000);
    lv_slider_set_value(cutoff_slider, 1200, LV_ANIM_OFF);
    lv_obj_set_width(cutoff_slider, 196);
    lv_obj_align(cutoff_slider, LV_ALIGN_TOP_LEFT, 28, 76);
    lv_obj_add_event_cb(cutoff_slider, LvglUi_CutoffChanged,
                       LV_EVENT_VALUE_CHANGED, cutoff_label);

    mix_label = lv_label_create(screen);
    lv_label_set_text(mix_label, "MIX 50 %");
    lv_obj_align(mix_label, LV_ALIGN_TOP_LEFT, 16, 116);

    mix_slider = lv_slider_create(screen);
    lv_slider_set_range(mix_slider, 0, 100);
    lv_slider_set_value(mix_slider, 50, LV_ANIM_OFF);
    lv_obj_set_width(mix_slider, 196);
    lv_obj_align(mix_slider, LV_ALIGN_TOP_LEFT, 28, 140);
    lv_obj_add_event_cb(mix_slider, LvglUi_MixChanged,
                       LV_EVENT_VALUE_CHANGED, mix_label);

    s_ui_ready = true;
    return 0;
}

void LvglUi_Service(void)
{
    if (s_ui_ready && g_lvgl_port_status == LVGL_PORT_READY) {
        (void)lv_timer_handler();
    }
    g_lvgl_ui_status = g_lvgl_port_status;
}
