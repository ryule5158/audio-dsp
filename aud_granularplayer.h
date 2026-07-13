#ifndef AUD_GRANULARPLAYER_H
#define AUD_GRANULARPLAYER_H
#include <stdint.h>
#include "aud_phasor.h"
typedef struct {float *sample;int size;float sample_rate,sample_frequency,grain_size,speed,transposition;
 float idxSpeed,idxSpeed2,idxTransp,idxTransp2,sig,sig2;uint32_t idx,idx2;float cosEnv[256];Aud_Phasor phs,phsImp,phs2,phsImp2;} Aud_GranularPlayer;
void Aud_GranularPlayer_Init(Aud_GranularPlayer *self,float*sample,int size,float sr);
float Aud_GranularPlayer_Process(Aud_GranularPlayer *self,float speed,float transposition,float grain_size);
#endif
