#ifndef AUD_TREMOLO_H
#define AUD_TREMOLO_H
#include <stdint.h>
#include "aud_osc.h"

typedef struct {
    float   sample_rate;
    float   dc_os;
    Aud_Osc osc;
} Aud_Tremolo;

void  Aud_Tremolo_Init(Aud_Tremolo *self, float sample_rate);
float Aud_Tremolo_Process(Aud_Tremolo *self, float in);
void  Aud_Tremolo_SetFreq(Aud_Tremolo *self, float freq);
void  Aud_Tremolo_SetWaveform(Aud_Tremolo *self, int waveform);
void  Aud_Tremolo_SetDepth(Aud_Tremolo *self, float depth);

#endif
