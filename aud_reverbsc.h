/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef AUD_REVERBSC_H
#define AUD_REVERBSC_H
#include <stdint.h>
#define AUD_REVERBSC_MAX_SIZE 98936

typedef struct {
    int write_pos, buffer_size, read_pos, read_pos_frac, read_pos_frac_inc, dummy, seed_val, rand_line_cnt;
    float filter_state, *buf;
} Aud_ReverbScDl;

typedef struct {
    float feedback, lpfreq, i_sample_rate, i_pitch_mod, i_skip_init, sample_rate, damp_fact, prv_lpfreq;
    int init_done;
    Aud_ReverbScDl delay_lines[8];
    float aux[AUD_REVERBSC_MAX_SIZE];
} Aud_ReverbSc;

int Aud_ReverbSc_Init(Aud_ReverbSc *self, float sr);
int Aud_ReverbSc_Process(Aud_ReverbSc *self, float in1, float in2, float *out1, float *out2);
void Aud_ReverbSc_SetFeedback(Aud_ReverbSc *self, float fb);
void Aud_ReverbSc_SetLpFreq(Aud_ReverbSc *self, float freq);
#endif
