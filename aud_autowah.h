#ifndef AUD_AUTOWAH_H
#define AUD_AUTOWAH_H
#include <stdint.h>

typedef struct {
    float sampling_freq, const1, const2, const4, wah, level, wet_dry;
    float rec0[3], rec1[2], rec2[2], rec3[2], rec4[2], rec5[2];
} Aud_Autowah;

void  Aud_Autowah_Init(Aud_Autowah *self, float sample_rate);
float Aud_Autowah_Process(Aud_Autowah *self, float in);
void  Aud_Autowah_SetWah(Aud_Autowah *self, float wah);
void  Aud_Autowah_SetDryWet(Aud_Autowah *self, float drywet);
void  Aud_Autowah_SetLevel(Aud_Autowah *self, float level);

#endif
