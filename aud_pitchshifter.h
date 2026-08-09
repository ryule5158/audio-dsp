#ifndef AUD_PITCHSHIFTER_H
#define AUD_PITCHSHIFTER_H

#include <stdbool.h>
#include <stdint.h>

#include "aud_delayline.h"

#define AUD_PITCHSHIFTER_MAX_DELAY 4096

typedef struct {
    float sample_rate;
    float shift;
    float feedback;
    float mix;
    float del_buf[AUD_PITCHSHIFTER_MAX_DELAY];
    Aud_DelayLine delay;
    float phase;
    float lfo_phase;
    float lfo_freq;
    float lfo_depth;
    bool enable_lfo;
} Aud_PitchShifter;

void Aud_PitchShifter_Init(Aud_PitchShifter *self,
                           float sample_rate,
                           float shift_semitones);
float Aud_PitchShifter_Process(Aud_PitchShifter *self, float in);
void Aud_PitchShifter_SetShift(Aud_PitchShifter *self, float semitones);
void Aud_PitchShifter_SetFeedback(Aud_PitchShifter *self, float feedback);
void Aud_PitchShifter_SetMix(Aud_PitchShifter *self, float mix);

#endif
