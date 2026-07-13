/**
 * @file    aud_looper.h
 * @brief   Multimode audio looper — Normal / OnetimeDub / Replace / Frippertronics.
 *          Ported from DaisySP Utility/looper (MIT)
 *          Original by Electrosmith, 2020. ~300 lines of header-only C++.
 */
#ifndef AUD_LOOPER_H
#define AUD_LOOPER_H
#include <stdint.h>
#include <stdbool.h>
#include "aud_dsp.h"
#define AUD_LOOPER_WINDOW_SAMPS 1200
#define AUD_LOOPER_WINDOW_FACTOR (1.0f/(float)AUD_LOOPER_WINDOW_SAMPS)
#define AUD_LOOPER_FRIP_DECAY 0.7071067811865476f
typedef enum {AUD_LOOP_NORMAL,AUD_LOOP_ONETIME_DUB,AUD_LOOP_REPLACE,AUD_LOOP_FRIPPERTRONICS} Aud_Looper_Mode;
typedef enum {AUD_LOOP_STATE_EMPTY,AUD_LOOP_STATE_REC_FIRST,AUD_LOOP_STATE_PLAYING,AUD_LOOP_STATE_REC_DUB} Aud_Looper_State;
typedef struct {Aud_Looper_Mode mode;Aud_Looper_State state;float*buff;uint32_t buffer_size;float pos,win_;uint32_t win_idx,recsize;
 bool half_speed,reverse,rec_queue,near_beginning;} Aud_Looper;
void Aud_Looper_Init(Aud_Looper *self,float*mem,uint32_t size);
float Aud_Looper_Process(Aud_Looper *self,float input);
void Aud_Looper_Clear(Aud_Looper *self);
void Aud_Looper_TrigRecord(Aud_Looper *self);
bool Aud_Looper_Recording(Aud_Looper *self);
void Aud_Looper_IncrementMode(Aud_Looper *self);
void Aud_Looper_SetMode(Aud_Looper *self,Aud_Looper_Mode m);
Aud_Looper_Mode Aud_Looper_GetMode(Aud_Looper *self);
void Aud_Looper_ToggleReverse(Aud_Looper *self);
void Aud_Looper_SetReverse(Aud_Looper *self,bool state);
bool Aud_Looper_GetReverse(Aud_Looper *self);
void Aud_Looper_ToggleHalfSpeed(Aud_Looper *self);
void Aud_Looper_SetHalfSpeed(Aud_Looper *self,bool state);
bool Aud_Looper_GetHalfSpeed(Aud_Looper *self);
bool Aud_Looper_IsNearBeginning(Aud_Looper *self);
#endif
