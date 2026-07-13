#ifndef AUD_DRIP_H
#define AUD_DRIP_H
#include <stdint.h>
#include <stdbool.h>
typedef struct {
 float gains0,gains1,gains2,kloop,dettack,num_tubes,damp,shake_max,freq,freq1,freq2,amp,snd_level;
 float outputs00,outputs01,outputs10,outputs11,outputs20,outputs21,total_energy;
 float center_freqs0,center_freqs1,center_freqs2,num_objects_save,sound_decay,system_decay;
 float finalZ0,finalZ1,finalZ2,coeffs01,coeffs00,coeffs11,coeffs10,coeffs21,coeffs20;
 float shake_energy,shake_damp,shake_max_save,num_objects,sample_rate,res_freq0,res_freq1,res_freq2,inputs1,inputs2;
} Aud_Drip;
void Aud_Drip_Init(Aud_Drip *self,float sample_rate,float dettack);
float Aud_Drip_Process(Aud_Drip *self,bool trig);
#endif
