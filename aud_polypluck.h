/**
 * @file    aud_polypluck.h
 * @brief   Polyphonic pluck — wraps multiple Pluck instances for polyphony.
 *          Ported from DaisySP PhysicalModeling/polypluck (MIT)
 */
#ifndef AUD_POLYPLUCK_H
#define AUD_POLYPLUCK_H
#include <stdint.h>
#include <stdbool.h>
#include "aud_pluck.h"
#include "aud_dcblock.h"
#define AUD_POLYPLUCK_MAX_VOICES 4
typedef struct {Aud_Pluck pluck[AUD_POLYPLUCK_MAX_VOICES];Aud_DcBlock dc[AUD_POLYPLUCK_MAX_VOICES];
 float buf[AUD_POLYPLUCK_MAX_VOICES][1024];bool active[AUD_POLYPLUCK_MAX_VOICES];int n_voices;} Aud_PolyPluck;
void Aud_PolyPluck_Init(Aud_PolyPluck *self,float sr,int n_voices,int mode);
float Aud_PolyPluck_Process(Aud_PolyPluck *self,float trig);
void Aud_PolyPluck_SetFreq(Aud_PolyPluck *self,int voice,float freq);
void Aud_PolyPluck_SetDecay(Aud_PolyPluck *self,int voice,float decay);
void Aud_PolyPluck_SetDamp(Aud_PolyPluck *self,int voice,float damp);
#endif
