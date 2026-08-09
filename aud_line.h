/* SPDX-License-Identifier: LGPL-2.1-only */
/**
 * @file    aud_line.h
 * @brief   Linear ramp generator (start→end over duration).
 *          Ported from DaisySP Control/line (MIT)
 */
#ifndef AUD_LINE_H
#define AUD_LINE_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float sample_rate;
    float dur;       /* Duration in seconds */
    float end;       /* Target value */
    float start;     /* Starting value */
    float val;       /* Current value */
    float inc;       /* Increment per sample */
    uint8_t finished;/* Done flag */
} Aud_Line;

void  Aud_Line_Init(Aud_Line *self, float sample_rate);
void  Aud_Line_Start(Aud_Line *self, float start, float end, float dur);
float Aud_Line_Process(Aud_Line *self, uint8_t *finished);

#endif /* AUD_LINE_H */
