#ifndef AUD_BLOSC_H
#define AUD_BLOSC_H
#include <stdint.h>
enum {AUD_BLOSC_WAVE_TRIANGLE,AUD_BLOSC_WAVE_SAW,AUD_BLOSC_WAVE_SQUARE,AUD_BLOSC_WAVE_OFF};
typedef struct {float rec0[2],rec1[2],vec0[2],vec1[2],vec2[4096],freq,amp,pw,sampling_freq,half_sr,quarter_sr,sec_per_sample,two_over_sr,four_over_sr;uint8_t mode;int iota;} Aud_BlOsc;
void Aud_BlOsc_Init(Aud_BlOsc *self,float sr);
float Aud_BlOsc_Process(Aud_BlOsc *self);
void Aud_BlOsc_Reset(Aud_BlOsc *self);
void Aud_BlOsc_SetFreq(Aud_BlOsc *self,float f);
void Aud_BlOsc_SetAmp(Aud_BlOsc *self,float a);
void Aud_BlOsc_SetPw(Aud_BlOsc *self,float pw);
void Aud_BlOsc_SetWaveform(Aud_BlOsc *self,uint8_t wf);
#endif
