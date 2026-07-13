/**
 * @file    aud_samplehold.h
 * @brief   Dual track-and-hold / sample-and-hold.
 *          Ported from DaisySP Utility/samplehold (MIT)
 *          Original by Paul Batchelor, 2015
 */
#ifndef AUD_SAMPLEHOLD_H
#define AUD_SAMPLEHOLD_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    AUD_SH_MODE_SAMPLE_HOLD,
    AUD_SH_MODE_TRACK_HOLD,
    AUD_SH_MODE_LAST
} Aud_SampleHold_Mode;

typedef struct {
    float track;
    float sample;
    bool  previous;
} Aud_SampleHold;

static inline void Aud_SampleHold_Init(Aud_SampleHold *self)
{
    if (self == NULL) return;
    self->track    = 0.0f;
    self->sample   = 0.0f;
    self->previous = false;
}

static inline float Aud_SampleHold_Process(Aud_SampleHold *self,
                                            bool trigger, float input,
                                            Aud_SampleHold_Mode mode)
{
    if (self == NULL) return 0.0f;
    if (trigger) {
        if (!self->previous) {
            self->sample = input;
        }
        self->track = input;
    }
    self->previous = trigger;
    return (mode == AUD_SH_MODE_SAMPLE_HOLD) ? self->sample : self->track;
}

#endif /* AUD_SAMPLEHOLD_H */
