#include "aud_soap.h"
#include <stddef.h>
#include <math.h>

#define AUD_SOAP_PI 3.141592653589793f

void Aud_Soap_Init(Aud_Soap *self, float sample_rate)
{
    if (self == NULL) return;
    self->center_freq   = 400.0f;
    self->bandwidth     = 50.0f;
    self->in_0          = 0.0f;
    self->din_1         = 0.0f;
    self->din_2         = 0.0f;
    self->dout_1        = 0.0f;
    self->dout_2        = 0.0f;
    self->all_output    = 0.0f;
    self->out_bandpass  = 0.0f;
    self->out_bandreject= 0.0f;
    self->sr            = sample_rate;
}

void Aud_Soap_Process(Aud_Soap *self, float in)
{
    if (self == NULL) return;
    float d = -cosf(2.0f * AUD_SOAP_PI * (self->center_freq / self->sr));
    float tf = tanf(AUD_SOAP_PI * (self->bandwidth / self->sr));
    float c = (tf - 1.0f) / (tf + 1.0f);

    self->in_0 = in;
    self->all_output = -c * self->in_0 + (d - d * c) * self->din_1
                     + self->din_2 - (d - d * c) * self->dout_1
                     + c * self->dout_2;

    self->din_2  = self->din_1;
    self->din_1  = self->in_0;
    self->dout_2 = self->dout_1;
    self->dout_1 = self->all_output;

    self->out_bandpass   = (self->in_0 + self->all_output * -1.0f) * 0.5f;
    self->out_bandreject = (self->in_0 + self->all_output * 0.99f) * 0.5f;
}

void Aud_Soap_SetCenterFreq(Aud_Soap *self, float f)
    { if (self) self->center_freq = f; }
void Aud_Soap_SetFilterBandwidth(Aud_Soap *self, float b)
    { if (self) self->bandwidth = b; }
float Aud_Soap_Bandpass(Aud_Soap *self)
    { return (self != NULL) ? self->out_bandpass : 0.0f; }
float Aud_Soap_Bandreject(Aud_Soap *self)
    { return (self != NULL) ? self->out_bandreject : 0.0f; }
