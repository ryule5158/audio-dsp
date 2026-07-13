#include <stddef.h>
#include "aud_osc.h"
#include "aud_dsp.h"
#include <math.h>

/* ---- internal: polyBLEP correction ---- */
static float aud_osc_polyblep(float phase_inc, float t)
{
    float dt = phase_inc;
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0f;
    } else if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

/* ---- lifecycle ---- */
void Aud_Osc_Init(Aud_Osc *osc, float sample_rate)
{
    if (osc == NULL) return;
    osc->sr        = sample_rate;
    osc->sr_recip  = 1.0f / sample_rate;
    osc->freq      = 100.0f;
    osc->amp       = 0.5f;
    osc->pw        = 0.5f;
    osc->phase     = 0.0f;
    osc->phase_inc = osc->freq * osc->sr_recip;
    osc->waveform  = AUD_WAVE_SIN;
    osc->last_out  = 0.0f;
    osc->eoc       = true;
    osc->eor       = true;
}

void Aud_Osc_SetFreq(Aud_Osc *osc, float freq)
{
    if (osc == NULL) return;
    osc->freq      = freq;
    osc->phase_inc = freq * osc->sr_recip;
}

void Aud_Osc_SetAmp(Aud_Osc *osc, float amp)
{
    if (osc == NULL) return;
    osc->amp = amp;
}

void Aud_Osc_SetWaveform(Aud_Osc *osc, uint8_t wf)
{
    if (osc == NULL) return;
    osc->waveform = (wf < AUD_WAVE_LAST) ? wf : AUD_WAVE_SIN;
}

void Aud_Osc_SetPw(Aud_Osc *osc, float pw)
{
    if (osc == NULL) return;
    osc->pw = aud_fclamp(pw, 0.0f, 1.0f);
}

void Aud_Osc_PhaseAdd(Aud_Osc *osc, float phase_delta)
{
    if (osc == NULL) return;
    osc->phase += phase_delta;
}

void Aud_Osc_Reset(Aud_Osc *osc, float phase)
{
    if (osc == NULL) return;
    osc->phase = phase;
}

/* ---- per-sample process ---- */
float Aud_Osc_Process(Aud_Osc *osc)
{
    if (osc == NULL) return 0.0f;

    float out, t;

    switch (osc->waveform) {
    case AUD_WAVE_SIN:
        out = sinf(osc->phase * AUD_TWOPI);
        break;

    case AUD_WAVE_TRI:
        t   = -1.0f + 2.0f * osc->phase;
        out = 2.0f * (fabsf(t) - 0.5f);
        break;

    case AUD_WAVE_SAW:
        out = -1.0f * ((osc->phase * 2.0f) - 1.0f);
        break;

    case AUD_WAVE_RAMP:
        out = (osc->phase * 2.0f) - 1.0f;
        break;

    case AUD_WAVE_SQUARE:
        out = (osc->phase < osc->pw) ? 1.0f : -1.0f;
        break;

    case AUD_WAVE_POLYBLEP_TRI:
        t   = osc->phase;
        out = (osc->phase < 0.5f) ? 1.0f : -1.0f;
        out += aud_osc_polyblep(osc->phase_inc, t);
        out -= aud_osc_polyblep(osc->phase_inc,
                                aud_fastmod1f(t + 0.5f));
        /* leaky integrator: y[n] = phase_inc*x[n] + (1-phase_inc)*y[n-1] */
        out          = osc->phase_inc * out
                     + (1.0f - osc->phase_inc) * osc->last_out;
        osc->last_out = out;
        out          *= 4.0f;  /* normalize after integration */
        break;

    case AUD_WAVE_POLYBLEP_SAW:
        t   = osc->phase;
        out = (2.0f * t) - 1.0f;
        out -= aud_osc_polyblep(osc->phase_inc, t);
        out *= -1.0f;
        break;

    case AUD_WAVE_POLYBLEP_SQUARE:
        t   = osc->phase;
        out = (osc->phase < osc->pw) ? 1.0f : -1.0f;
        out += aud_osc_polyblep(osc->phase_inc, t);
        out -= aud_osc_polyblep(osc->phase_inc,
                                aud_fastmod1f(t + (1.0f - osc->pw)));
        out *= 0.707f;
        break;

    default:
        out = 0.0f;
        break;
    }

    /* advance phase */
    osc->phase += osc->phase_inc;
    if (osc->phase > 1.0f) {
        osc->phase -= 1.0f;
        osc->eoc = true;
    } else {
        osc->eoc = false;
    }
    osc->eor = (osc->phase - osc->phase_inc < 0.5f)
            && (osc->phase >= 0.5f);

    return out * osc->amp;
}
