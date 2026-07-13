#include "aud_overdrive.h"
#include <stddef.h>
#include "aud_dsp.h"

void Aud_Overdrive_Init(Aud_Overdrive *self)
{
    if (self == NULL) return;
    Aud_Overdrive_SetDrive(self, 0.5f);
}

float Aud_Overdrive_Process(Aud_Overdrive *self, float in)
{
    if (self == NULL) return in;
    float pre = self->pre_gain * in;
    return aud_soft_clip(pre) * self->post_gain;
}

void Aud_Overdrive_SetDrive(Aud_Overdrive *self, float drive)
{
    if (self == NULL) return;
    drive  = aud_fclamp(drive, 0.0f, 1.0f);
    self->drive = 2.0f * drive;

    const float drive_2    = self->drive * self->drive;
    const float pre_gain_a = self->drive * 0.5f;
    const float pre_gain_b = drive_2 * drive_2 * self->drive * 24.0f;
    self->pre_gain         = pre_gain_a + (pre_gain_b - pre_gain_a) * drive_2;

    const float drive_squashed = self->drive * (2.0f - self->drive);
    self->post_gain = 1.0f / aud_soft_clip(0.33f
        + drive_squashed * (self->pre_gain - 0.33f));
}
