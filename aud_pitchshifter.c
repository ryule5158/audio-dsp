#include "aud_pitchshifter.h"

#include "aud_dsp.h"

#include <math.h>
#include <stddef.h>

#define AUD_PITCH_MIN_DELAY 2.0f

static float aud_pitch_wrap(float phase)
{
    while (phase >= 1.0f) {
        phase -= 1.0f;
    }
    while (phase < 0.0f) {
        phase += 1.0f;
    }
    return phase;
}

void Aud_PitchShifter_Init(Aud_PitchShifter *self, float sr, float shift)
{
    if (self == NULL) {
        return;
    }
    self->sample_rate = sr;
    self->shift = aud_fclamp(shift, -24.0f, 24.0f);
    self->feedback = 0.0f;
    self->mix = 0.5f;
    self->phase = 0.0f;
    self->lfo_phase = 0.0f;
    self->lfo_freq = 0.0f;
    self->lfo_depth = 0.0f;
    self->enable_lfo = false;
    Aud_DelayLine_Init(&self->delay, self->del_buf,
                       AUD_PITCHSHIFTER_MAX_DELAY);
}

float Aud_PitchShifter_Process(Aud_PitchShifter *self, float in)
{
    float ratio;
    float sweep_samples;
    float phase_b;
    float delay_a;
    float delay_b;
    float weight_a;
    float weight_b;
    float modulation = 0.0f;
    float shifted;

    if (self == NULL) {
        return in;
    }

    ratio = powf(2.0f, self->shift / 12.0f);
    sweep_samples = (float)AUD_PITCHSHIFTER_MAX_DELAY -
        (AUD_PITCH_MIN_DELAY + 2.0f);
    self->phase = aud_pitch_wrap(
        self->phase + (1.0f - ratio) / sweep_samples);
    phase_b = aud_pitch_wrap(self->phase + 0.5f);

    if (self->enable_lfo) {
        modulation = sinf(self->lfo_phase) * self->lfo_depth;
        self->lfo_phase += self->lfo_freq;
        if (self->lfo_phase >= AUD_TWOPI) {
            self->lfo_phase -= AUD_TWOPI;
        }
    }

    delay_a = AUD_PITCH_MIN_DELAY + self->phase * sweep_samples + modulation;
    delay_b = AUD_PITCH_MIN_DELAY + phase_b * sweep_samples + modulation;
    delay_a = aud_fclamp(delay_a, AUD_PITCH_MIN_DELAY,
                         (float)AUD_PITCHSHIFTER_MAX_DELAY - 2.0f);
    delay_b = aud_fclamp(delay_b, AUD_PITCH_MIN_DELAY,
                         (float)AUD_PITCHSHIFTER_MAX_DELAY - 2.0f);

    /* sin^2/cos^2 windows are complementary for heads 180 degrees apart. */
    weight_a = sinf(AUD_PI * self->phase);
    weight_a *= weight_a;
    weight_b = 1.0f - weight_a;
    shifted = Aud_DelayLine_ReadAt(&self->delay, delay_a) * weight_a +
        Aud_DelayLine_ReadAt(&self->delay, delay_b) * weight_b;
    Aud_DelayLine_Write(&self->delay, in + shifted * self->feedback);
    return in * (1.0f - self->mix) + shifted * self->mix;
}

void Aud_PitchShifter_SetShift(Aud_PitchShifter *self, float semitones)
{
    if (self != NULL) {
        self->shift = aud_fclamp(semitones, -24.0f, 24.0f);
    }
}

void Aud_PitchShifter_SetFeedback(Aud_PitchShifter *self, float feedback)
{
    if (self != NULL) {
        self->feedback = aud_fclamp(feedback, -0.95f, 0.95f);
    }
}

void Aud_PitchShifter_SetMix(Aud_PitchShifter *self, float mix)
{
    if (self != NULL) {
        self->mix = aud_fclamp(mix, 0.0f, 1.0f);
    }
}
