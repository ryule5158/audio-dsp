#ifndef AUD_RESONATOR_H
#define AUD_RESONATOR_H
#include <stdint.h>
#define AUD_RESONATOR_MAX_MODES 24
#define AUD_RESONATOR_BATCH_SIZE 4
typedef struct {float state_1[AUD_RESONATOR_BATCH_SIZE],state_2[AUD_RESONATOR_BATCH_SIZE];} Aud_ResonatorSvf;
typedef struct {int resolution;float frequency,brightness,structure,damping,sample_rate;
 float mode_amplitude[AUD_RESONATOR_MAX_MODES];Aud_ResonatorSvf mode_filters[AUD_RESONATOR_MAX_MODES/AUD_RESONATOR_BATCH_SIZE];} Aud_Resonator;
void Aud_ResonatorSvf_Init(Aud_ResonatorSvf *self);
void Aud_Resonator_Init(Aud_Resonator *self,float position,int resolution,float sr);
float Aud_Resonator_Process(Aud_Resonator *self,float in);
void Aud_Resonator_SetFreq(Aud_Resonator *self,float f);
void Aud_Resonator_SetStructure(Aud_Resonator *self,float s);
void Aud_Resonator_SetBrightness(Aud_Resonator *self,float b);
void Aud_Resonator_SetDamping(Aud_Resonator *self,float d);
#endif
