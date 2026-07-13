#ifndef AUD_MODALVOICE_H
#define AUD_MODALVOICE_H
#include <stdint.h>
#include <stdbool.h>
#include "aud_resonator.h"
#include "aud_dust.h"
typedef struct {float sample_rate,f0,structure,brightness,damping,density,accent,aux;bool sustain,trig;
 Aud_ResonatorSvf excitation_filter;Aud_Resonator resonator;Aud_Dust dust;} Aud_ModalVoice;
void Aud_ModalVoice_Init(Aud_ModalVoice *self,float sr);
float Aud_ModalVoice_Process(Aud_ModalVoice *self,bool trigger);
void Aud_ModalVoice_SetSustain(Aud_ModalVoice *self,bool s);
void Aud_ModalVoice_Trig(Aud_ModalVoice *self);
void Aud_ModalVoice_SetFreq(Aud_ModalVoice *self,float f);
void Aud_ModalVoice_SetAccent(Aud_ModalVoice *self,float a);
void Aud_ModalVoice_SetStructure(Aud_ModalVoice *self,float s);
void Aud_ModalVoice_SetBrightness(Aud_ModalVoice *self,float b);
void Aud_ModalVoice_SetDamping(Aud_ModalVoice *self,float d);
float Aud_ModalVoice_GetAux(Aud_ModalVoice *self);
#endif
