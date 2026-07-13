#include "aud_line.h"
#include <stddef.h>
#include "aud_dsp.h"

void Aud_Line_Init(Aud_Line *self, float sample_rate)
{
    if (self == NULL) return;
    self->sample_rate = sample_rate;
    self->dur         = 0.5f;
    self->end         = 0.0f;
    self->start       = 1.0f;
    self->val         = 1.0f;
}

void Aud_Line_Start(Aud_Line *self, float start, float end, float dur)
{
    if (self == NULL) return;
    self->start    = start;
    self->end      = end;
    self->dur      = dur;
    self->inc      = (end - start) / (self->sample_rate * dur);
    self->val      = start;
    self->finished = 0;
}

float Aud_Line_Process(Aud_Line *self, uint8_t *finished)
{
    if (self == NULL) {
        if (finished) *finished = 0;
        return 0.0f;
    }
    float out = self->val;

    if ((self->end > self->start && out >= self->end)
     || (self->end < self->start && out <= self->end)) {
        self->finished = 1;
        self->val      = self->end;
        out            = self->end;
    } else {
        self->val += self->inc;
    }
    if (finished) *finished = self->finished;
    return out;
}
