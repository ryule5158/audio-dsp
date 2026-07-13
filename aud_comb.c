#include <stddef.h>
#include "aud_comb.h"
#include <string.h>

void Aud_Comb_Init(Aud_Comb *c, uint32_t max_delay)
{
    if (c == NULL) return;
    if (max_delay > 4095u) max_delay = 4095u;
    c->max_size = max_delay + 1u;
    c->delay    = max_delay;
    c->gain     = 0.7f;
    c->write_ptr = 0u;
    c->rev       = 1u;
    memset(c->buffer, 0, sizeof(c->buffer));
}

void Aud_Comb_SetDelay(Aud_Comb *c, uint32_t delay)
{
    if (c == NULL) return;
    c->delay = (delay < c->max_size) ? delay : c->max_size - 1u;
}

void Aud_Comb_SetGain(Aud_Comb *c, float gain)
{
    if (c == NULL) return;
    c->gain = gain;
}

void Aud_Comb_SetRev(Aud_Comb *c, uint8_t reverse)
{
    if (c == NULL) return;
    c->rev = (reverse != 0u) ? 1u : 0u;
}

float Aud_Comb_Process(Aud_Comb *c, float in)
{
    if (c == NULL) return in;
    uint32_t rd = (c->write_ptr + c->delay) % c->max_size;
    float read   = c->buffer[rd];
    float write  = c->rev ? (in + c->gain * read)
                          : (in);
    c->buffer[c->write_ptr] = write;
    c->write_ptr = (c->write_ptr == 0u)
                   ? (c->max_size - 1u) : (c->write_ptr - 1u);
    return c->rev ? read : (in + c->gain * read);
}
