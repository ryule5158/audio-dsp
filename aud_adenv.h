/**
 * @file    aud_adenv.h
 * @brief   Attack-Decay envelope — triggerable, with min/max range and curve shape.
 *          Ported from DaisySP Control/adenv (MIT)
 */
#ifndef AUD_ADENV_H
#define AUD_ADENV_H
#include <stdint.h>
#include <stdbool.h>

enum { AUD_ADENV_SEG_IDLE, AUD_ADENV_SEG_ATTACK, AUD_ADENV_SEG_DECAY, AUD_ADENV_SEG_LAST };

typedef struct {
    uint8_t  current_segment, prev_segment;
    float    segment_time[AUD_ADENV_SEG_LAST];
    float    sample_rate, min, max, output, curve_scalar;
    float    c_inc, curve_x, retrig_val;
    uint32_t phase;
    uint8_t  trigger;
} Aud_AdEnv;

void  Aud_AdEnv_Init(Aud_AdEnv *self, float sample_rate);
float Aud_AdEnv_Process(Aud_AdEnv *self);
void  Aud_AdEnv_Trigger(Aud_AdEnv *self);
void  Aud_AdEnv_SetTime(Aud_AdEnv *self, uint8_t seg, float time);
void  Aud_AdEnv_SetCurve(Aud_AdEnv *self, float scalar);
void  Aud_AdEnv_SetMin(Aud_AdEnv *self, float min);
void  Aud_AdEnv_SetMax(Aud_AdEnv *self, float max);
float Aud_AdEnv_GetValue(const Aud_AdEnv *self);
uint8_t Aud_AdEnv_GetCurrentSegment(const Aud_AdEnv *self);
bool Aud_AdEnv_IsRunning(const Aud_AdEnv *self);

#endif
