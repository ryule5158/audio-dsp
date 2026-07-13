#include <stddef.h>
#include "aud_port.h"
#include <math.h>

void Aud_Port_Init(Aud_Port *p, float sample_rate, float half_time_sec)
{
    if (p == NULL) return;
    p->sample_rate = sample_rate;
    p->y1 = 0.0f;
    Aud_Port_SetHtime(p, half_time_sec);
}

void Aud_Port_SetHtime(Aud_Port *p, float half_time_sec)
{
    if (p == NULL) return;
    p->htime = half_time_sec;
    /* c1 = 1 - exp(-ln(2) / (htime * sr)) ≈ ln(2) / (htime*sr) for large sr */
    p->c1 = 1.0f - expf(-0.69314718056f / (half_time_sec * p->sample_rate));
}

float Aud_Port_Process(Aud_Port *p, float in)
{
    if (p == NULL) return in;
    p->y1 += p->c1 * (in - p->y1);
    return p->y1;
}
