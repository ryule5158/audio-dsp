#include "aud_nlfilt.h"
#include <stddef.h>
#include <math.h>
#include <string.h>

void Aud_NlFilt_Init(Aud_NlFilt *self)
{
    if (self == NULL) return;
    self->a = self->b = self->d = self->C = self->L = 0.0f;
    self->in = 0.0f;
    self->point = 0;
    memset(self->delay, 0, sizeof(self->delay));
}

void Aud_NlFilt_SetCoefficients(Aud_NlFilt *self, float a, float b, float d, float C, float L)
{
    if (self == NULL) return;
    self->a = a; self->b = b; self->d = d; self->C = C; self->L = L;
}

void Aud_NlFilt_SetA(Aud_NlFilt *self, float a) { if (self) self->a = a; }
void Aud_NlFilt_SetB(Aud_NlFilt *self, float b) { if (self) self->b = b; }
void Aud_NlFilt_SetD(Aud_NlFilt *self, float d) { if (self) self->d = d; }
void Aud_NlFilt_SetC(Aud_NlFilt *self, float C) { if (self) self->C = C; }
void Aud_NlFilt_SetL(Aud_NlFilt *self, float L) { if (self) self->L = L; }

void Aud_NlFilt_ProcessBlock(Aud_NlFilt *self, float *in, float *out, uint32_t size)
{
    if (self == NULL || in == NULL || out == NULL) return;
    float a = self->a, b = self->b, d = self->d;
    float C = self->C, L = self->L;
    int32_t point = self->point;

    for (uint32_t i = 0; i < size; i++) {
        float yn;
        int32_t idx = point - (int32_t)L;
        if (idx < 0) idx += AUD_NLFILT_MAX_DELAY;
        yn = tanhf(a * self->delay[(point - 1 + AUD_NLFILT_MAX_DELAY) % AUD_NLFILT_MAX_DELAY]
                 + b * self->delay[(point - 2 + AUD_NLFILT_MAX_DELAY) % AUD_NLFILT_MAX_DELAY]
                 + d * self->delay[idx] * self->delay[idx]
                 + in[i] - C);
        self->delay[point] = yn;
        out[i] = yn;
        point = (point + 1) % AUD_NLFILT_MAX_DELAY;
    }
    self->point = point;
}
