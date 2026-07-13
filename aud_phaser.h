#ifndef AUD_PHASER_H
#define AUD_PHASER_H
#include <stdint.h>
#include "aud_delayline.h"
#define AUD_PHASER_DELAY_LEN 2400
#define AUD_PHASER_MAX_POLES 8

typedef struct {
    float sample_rate;
    float lfo_phase, lfo_freq, lfo_amp;
    float os, feedback, ap_freq, deltime, last_sample;
    float del_buf[AUD_PHASER_DELAY_LEN];
    Aud_DelayLine del;
} Aud_PhaserEngine;

typedef struct {
    Aud_PhaserEngine engines[AUD_PHASER_MAX_POLES];
    float gain_frac;
    int   poles;
} Aud_Phaser;

void  Aud_PhaserEngine_Init(Aud_PhaserEngine *self, float sr);
float Aud_PhaserEngine_Process(Aud_PhaserEngine *self, float in);
void  Aud_PhaserEngine_SetLfoDepth(Aud_PhaserEngine *self, float d);
void  Aud_PhaserEngine_SetLfoFreq(Aud_PhaserEngine *self, float f);
void  Aud_PhaserEngine_SetFreq(Aud_PhaserEngine *self, float f);
void  Aud_PhaserEngine_SetFeedback(Aud_PhaserEngine *self, float fb);

void  Aud_Phaser_Init(Aud_Phaser *self, float sr);
float Aud_Phaser_Process(Aud_Phaser *self, float in);
void  Aud_Phaser_SetPoles(Aud_Phaser *self, int poles);
void  Aud_Phaser_SetLfoDepth(Aud_Phaser *self, float d);
void  Aud_Phaser_SetLfoFreq(Aud_Phaser *self, float f);
void  Aud_Phaser_SetFreq(Aud_Phaser *self, float f);
void  Aud_Phaser_SetFeedback(Aud_Phaser *self, float fb);
#endif
