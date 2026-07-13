#include <stddef.h>
#include "aud_whitenoise.h"

void Aud_WhiteNoise_Init(Aud_WhiteNoise *wn, uint32_t seed)
{
    if (wn == NULL) return;
    wn->seed = seed;
}

float Aud_WhiteNoise_Process(Aud_WhiteNoise *wn)
{
    if (wn == NULL) return 0.0f;
    /* 32-bit LCG: X_n+1 = (1664525 * X_n + 1013904223) mod 2^32 */
    wn->seed = 1664525u * wn->seed + 1013904223u;
    /* scale to [-1, 1) */
    return ((float)(wn->seed >> 1u) / 2147483648.0f) - 1.0f;
}
