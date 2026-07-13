/**
 * @file    aud_osc.h
 * @brief   Multi-waveform oscillator with polyBLEP bandlimiting.
 *          Ported from DaisySP Synthesis/oscillator (MIT)
 *
 *          Waveforms:
 *            AUD_WAVE_SIN              - naive sinf()
 *            AUD_WAVE_TRI              - naive triangle
 *            AUD_WAVE_SAW              - naive sawtooth
 *            AUD_WAVE_RAMP             - naive reverse saw
 *            AUD_WAVE_SQUARE           - naive square (w/ pulse width)
 *            AUD_WAVE_POLYBLEP_TRI     - bandlimited triangle
 *            AUD_WAVE_POLYBLEP_SAW     - bandlimited sawtooth
 *            AUD_WAVE_POLYBLEP_SQUARE  - bandlimited square
 */
#ifndef AUD_OSC_H
#define AUD_OSC_H
#include <stdint.h>
#include <stdbool.h>

enum {
    AUD_WAVE_SIN = 0,
    AUD_WAVE_TRI,
    AUD_WAVE_SAW,
    AUD_WAVE_RAMP,
    AUD_WAVE_SQUARE,
    AUD_WAVE_POLYBLEP_TRI,
    AUD_WAVE_POLYBLEP_SAW,
    AUD_WAVE_POLYBLEP_SQUARE,
    AUD_WAVE_LAST
};

typedef struct {
    uint8_t waveform;     /* Waveform selection (AUD_WAVE_xxx) */
    float   amp;          /* Output amplitude */
    float   freq;         /* Frequency (Hz) */
    float   pw;           /* Pulse width [0, 1] for square waves */
    float   sr;           /* Sample rate (Hz) */
    float   sr_recip;     /* 1 / sample_rate */
    float   phase;        /* Current phase [0, 1) */
    float   phase_inc;    /* Phase increment per sample = freq / sr */
    float   last_out;     /* Previous output (for polyBLEP TRI integrator) */
    bool    eoc;          /* End of cycle flag */
    bool    eor;          /* End of rise flag */
} Aud_Osc;

/* ---- Lifecycle ---- */
void Aud_Osc_Init(Aud_Osc *osc, float sample_rate);
void Aud_Osc_SetFreq(Aud_Osc *osc, float freq);
void Aud_Osc_SetAmp(Aud_Osc *osc, float amp);
void Aud_Osc_SetWaveform(Aud_Osc *osc, uint8_t wf);
void Aud_Osc_SetPw(Aud_Osc *osc, float pw);
void Aud_Osc_PhaseAdd(Aud_Osc *osc, float phase_delta);
void Aud_Osc_Reset(Aud_Osc *osc, float phase);

/* ---- Per-sample process ---- */
float Aud_Osc_Process(Aud_Osc *osc);

/* ---- State queries ---- */
static inline bool Aud_Osc_IsEOR(const Aud_Osc *osc)    { return osc->eor; }
static inline bool Aud_Osc_IsEOC(const Aud_Osc *osc)    { return osc->eoc; }
static inline bool Aud_Osc_IsRising(const Aud_Osc *osc) { return osc->phase < 0.5f; }
static inline bool Aud_Osc_IsFalling(const Aud_Osc *osc){ return osc->phase >= 0.5f; }

#endif /* AUD_OSC_H */
