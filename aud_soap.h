#ifndef AUD_SOAP_H
#define AUD_SOAP_H
#include <stdint.h>

typedef struct {
    float center_freq, bandwidth;
    float in_0, din_1, din_2, dout_1, dout_2;
    float all_output, out_bandpass, out_bandreject;
    float sr;
} Aud_Soap;

void  Aud_Soap_Init(Aud_Soap *self, float sample_rate);
void  Aud_Soap_Process(Aud_Soap *self, float in);
void  Aud_Soap_SetCenterFreq(Aud_Soap *self, float f);
void  Aud_Soap_SetFilterBandwidth(Aud_Soap *self, float b);
float Aud_Soap_Bandpass(Aud_Soap *self);
float Aud_Soap_Bandreject(Aud_Soap *self);

#endif
