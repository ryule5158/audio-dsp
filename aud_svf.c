#include <stddef.h>
#include "aud_svf.h"
#include "aud_dsp.h"
#include <math.h>

void Aud_Svf_Init(Aud_Svf *svf, float sample_rate)
{
    if (svf == NULL) return;
    svf->sr        = sample_rate;
    svf->fc        = 200.0f;
    svf->res       = 0.5f;
    svf->drive     = 0.5f;
    svf->pre_drive = 0.5f;
    svf->freq      = 0.25f;
    svf->damp      = 0.0f;
    svf->notch     = 0.0f;
    svf->low       = 0.0f;
    svf->high      = 0.0f;
    svf->band      = 0.0f;
    svf->peak      = 0.0f;
    svf->out_low   = 0.0f;
    svf->out_high  = 0.0f;
    svf->out_band  = 0.0f;
    svf->out_peak  = 0.0f;
    svf->out_notch = 0.0f;
    svf->fc_max    = sample_rate / 3.0f;
}

void Aud_Svf_Process(Aud_Svf *svf, float in)
{
    if (svf == NULL) return;

    /* ---- pass 1 ---- */
    svf->notch = in - svf->damp * svf->band;
    svf->low   = svf->low + svf->freq * svf->band;
    svf->high  = svf->notch - svf->low;
    svf->band  = svf->freq * svf->high + svf->band
               - svf->drive * svf->band * svf->band * svf->band;
    /* sample output */
    svf->out_low   = 0.5f * svf->low;
    svf->out_high  = 0.5f * svf->high;
    svf->out_band  = 0.5f * svf->band;
    svf->out_peak  = 0.5f * (svf->low - svf->high);
    svf->out_notch = 0.5f * svf->notch;

    /* ---- pass 2 (double-sampled) ---- */
    svf->notch = in - svf->damp * svf->band;
    svf->low   = svf->low + svf->freq * svf->band;
    svf->high  = svf->notch - svf->low;
    svf->band  = svf->freq * svf->high + svf->band
               - svf->drive * svf->band * svf->band * svf->band;
    /* average with first pass */
    svf->out_low   += 0.5f * svf->low;
    svf->out_high  += 0.5f * svf->high;
    svf->out_band  += 0.5f * svf->band;
    svf->out_peak  += 0.5f * (svf->low - svf->high);
    svf->out_notch += 0.5f * svf->notch;
}

void Aud_Svf_SetFreq(Aud_Svf *svf, float f)
{
    if (svf == NULL) return;
    svf->fc = aud_fclamp(f, 1.0e-6f, svf->fc_max);
    svf->freq = 2.0f
        * sinf(AUD_PI
               * AUD_MIN(0.25f, svf->fc / (svf->sr * 2.0f)));
    svf->damp = AUD_MIN(2.0f * (1.0f - powf(svf->res, 0.25f)),
                        AUD_MIN(2.0f, 2.0f / svf->freq
                                      - svf->freq * 0.5f));
}

void Aud_Svf_SetRes(Aud_Svf *svf, float r)
{
    if (svf == NULL) return;
    svf->res = aud_fclamp(r, 0.0f, 1.0f);
    svf->damp = AUD_MIN(2.0f * (1.0f - powf(svf->res, 0.25f)),
                        AUD_MIN(2.0f, 2.0f / svf->freq
                                      - svf->freq * 0.5f));
    svf->drive = svf->pre_drive * svf->res;
}

void Aud_Svf_SetDrive(Aud_Svf *svf, float d)
{
    if (svf == NULL) return;
    svf->pre_drive = aud_fclamp(d * 0.1f, 0.0f, 1.0f);
    svf->drive     = svf->pre_drive * svf->res;
}
