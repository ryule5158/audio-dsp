/* SPDX-License-Identifier: LGPL-2.1-only */
/**
 * @file    aud_port.h
 * @brief   Portamento / slew limiter — smooths abrupt value changes.
 *          Ported from DaisySP Utility/port (MIT)
 */
#ifndef AUD_PORT_H
#define AUD_PORT_H
#include <stdint.h>

typedef struct {
    float sample_rate;
    float htime;    /* Half-time in seconds */
    float c1;       /* Filter coefficient */
    float y1;       /* State: previous output */
} Aud_Port;

void Aud_Port_Init(Aud_Port *p, float sample_rate, float half_time_sec);
void Aud_Port_SetHtime(Aud_Port *p, float half_time_sec);
float Aud_Port_Process(Aud_Port *p, float in);

#endif /* AUD_PORT_H */
