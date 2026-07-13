/**
 * @file    aud_adsr.h
 * @brief   ADSR envelope with exponential segments, gate-based triggering, and attack shape.
 *          Ported from DaisySP Control/adsr (MIT)
 *          Original by Paul Batchelor (Soundpipe), remade by Steffan Diedrichsen, 2021
 */
#ifndef AUD_ADSR_H
#define AUD_ADSR_H
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

enum { AUD_ADSR_SEG_IDLE = 0, AUD_ADSR_SEG_ATTACK = 1, AUD_ADSR_SEG_DECAY = 2, AUD_ADSR_SEG_RELEASE = 4 };

typedef struct {
    float   sus_level;
    float   x;
    float   attackShape;
    float   attackTarget;
    float   attackTime;
    float   decayTime;
    float   releaseTime;
    float   attackD0;
    float   decayD0;
    float   releaseD0;
    int     sample_rate;
    uint8_t mode;
    bool    gate;
} Aud_Adsr;

void  Aud_Adsr_Init(Aud_Adsr *self, float sample_rate, int blockSize);
float Aud_Adsr_Process(Aud_Adsr *self, bool gate);
void  Aud_Adsr_Retrigger(Aud_Adsr *self, bool hard);
void  Aud_Adsr_SetTime(Aud_Adsr *self, int seg, float time);
void  Aud_Adsr_SetAttackTime(Aud_Adsr *self, float timeInS, float shape);
void  Aud_Adsr_SetDecayTime(Aud_Adsr *self, float timeInS);
void  Aud_Adsr_SetReleaseTime(Aud_Adsr *self, float timeInS);
void  Aud_Adsr_SetSustainLevel(Aud_Adsr *self, float sus_level);
uint8_t Aud_Adsr_GetCurrentSegment(const Aud_Adsr *self);
bool Aud_Adsr_IsRunning(const Aud_Adsr *self);

#endif
