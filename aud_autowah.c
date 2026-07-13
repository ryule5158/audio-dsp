#include "aud_autowah.h"
#include <stddef.h>
#include <math.h>

void Aud_Autowah_Init(Aud_Autowah *self, float sample_rate)
{
    if (self == NULL) return;
    self->sampling_freq = sample_rate;
    self->const1        = 1413.72f / sample_rate;
    self->const2        = expf(0.0f - (100.0f / sample_rate));
    self->const4        = expf(0.0f - (10.0f / sample_rate));
    self->wet_dry       = 100.0f;
    self->level         = 0.1f;
    self->wah           = 0.0f;

    for (int i = 0; i < 2; i++) {
        self->rec1[i] = self->rec2[i] = self->rec3[i] = 0.0f;
        self->rec4[i] = self->rec5[i] = 0.0f;
    }
    for (int i = 0; i < 3; i++) {
        self->rec0[i] = 0.0f;
    }
}

void Aud_Autowah_SetWah(Aud_Autowah *self, float wah)
    { if (self) self->wah = wah; }
void Aud_Autowah_SetDryWet(Aud_Autowah *self, float drywet)
    { if (self) self->wet_dry = drywet; }
void Aud_Autowah_SetLevel(Aud_Autowah *self, float level)
    { if (self) self->level = level; }

float Aud_Autowah_Process(Aud_Autowah *self, float in)
{
    if (self == NULL) return in;
    float fSlow2 = (0.01f * (self->wet_dry * self->level));
    float fSlow3 = (1.0f - 0.01f * self->wet_dry) + (1.0f - self->wah);

    float fTemp1 = fabsf(in);
    self->rec3[0] = fmaxf(fTemp1,
        (self->const4 * self->rec3[1]) + ((1.0f - self->const4) * fTemp1));
    self->rec2[0] = (self->const2 * self->rec2[1])
                  + ((1.0f - self->const2) * self->rec3[0]);
    float fTemp2 = fminf(1.0f, self->rec2[0]);
    float fTemp3 = powf(2.0f, (2.3f * fTemp2));
    float fTemp4 = 1.0f - (self->const1 * fTemp3
        / powf(2.0f, (1.0f + 2.0f * (1.0f - fTemp2))));
    self->rec1[0] = ((0.999f * self->rec1[1])
        + (0.001f * (0.0f - (2.0f
            * (fTemp4 * cosf((self->const1 * 2.0f * fTemp3)))))));
    self->rec4[0] = ((0.999f * self->rec4[1])
                   + (0.001f * fTemp4 * fTemp4));
    self->rec5[0] = ((0.999f * self->rec5[1])
                   + (0.0001f * powf(4.0f, fTemp2)));
    self->rec0[0] = (0.0f
        - (((self->rec1[0] * self->rec0[1])
          + (self->rec4[0] * self->rec0[2]))
           - (fSlow2 * (self->rec5[0] * in))));

    float out = ((self->wah * (self->rec0[0] - self->rec0[1]))
               + (fSlow3 * in));
    self->rec3[1] = self->rec3[0];
    self->rec2[1] = self->rec2[0];
    self->rec1[1] = self->rec1[0];
    self->rec4[1] = self->rec4[0];
    self->rec5[1] = self->rec5[0];
    self->rec0[2] = self->rec0[1];
    self->rec0[1] = self->rec0[0];

    return out;
}
