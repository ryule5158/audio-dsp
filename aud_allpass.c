/* SPDX-License-Identifier: LGPL-2.1-only */
#include <stddef.h>
#include "aud_allpass.h"
#include <string.h>

void Aud_Allpass_Init(Aud_Allpass *ap, uint32_t max_delay)
{
    if (ap == NULL) return;
    if (max_delay > 2047u) max_delay = 2047u;
    ap->max_size = max_delay + 1u;
    ap->delay    = max_delay;
    ap->gain     = 0.7f;
    ap->write_ptr = 0u;
    memset(ap->buffer, 0, sizeof(ap->buffer));
}

void Aud_Allpass_SetDelay(Aud_Allpass *ap, uint32_t delay)
{
    if (ap == NULL) return;
    ap->delay = (delay < ap->max_size) ? delay : ap->max_size - 1u;
}

void Aud_Allpass_SetGain(Aud_Allpass *ap, float gain)
{
    if (ap == NULL) return;
    ap->gain = gain;
}

float Aud_Allpass_Process(Aud_Allpass *ap, float in)
{
    if (ap == NULL) return in;
    uint32_t rd = (ap->write_ptr + ap->delay) % ap->max_size;
    float read   = ap->buffer[rd];
    float write  = in + ap->gain * read;
    ap->buffer[ap->write_ptr] = write;
    ap->write_ptr = (ap->write_ptr == 0u)
                    ? (ap->max_size - 1u) : (ap->write_ptr - 1u);
    return -ap->gain * write + read;
}
