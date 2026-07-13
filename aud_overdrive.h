#ifndef AUD_OVERDRIVE_H
#define AUD_OVERDRIVE_H
#include <stdint.h>

typedef struct {
    float drive;
    float pre_gain;
    float post_gain;
} Aud_Overdrive;

void  Aud_Overdrive_Init(Aud_Overdrive *self);
float Aud_Overdrive_Process(Aud_Overdrive *self, float in);
void  Aud_Overdrive_SetDrive(Aud_Overdrive *self, float drive);

#endif
