#ifndef AUD_FLANGER_H
#define AUD_FLANGER_H
#include <stdint.h>
#include "aud_delayline.h"
#define AUD_FLANGER_DELAY_LEN 960

typedef struct {
    float sample_rate, feedback;
    float lfo_phase, lfo_freq, lfo_amp, delay;
    float del_buf[AUD_FLANGER_DELAY_LEN];
    Aud_DelayLine del;
} Aud_Flanger;

void  Aud_Flanger_Init(Aud_Flanger *self, float sr);
float Aud_Flanger_Process(Aud_Flanger *self, float in);
void  Aud_Flanger_SetFeedback(Aud_Flanger *self, float fb);
void  Aud_Flanger_SetLfoDepth(Aud_Flanger *self, float depth);
void  Aud_Flanger_SetLfoFreq(Aud_Flanger *self, float freq);
void  Aud_Flanger_SetDelay(Aud_Flanger *self, float delay);
void  Aud_Flanger_SetDelayMs(Aud_Flanger *self, float ms);
#endif
