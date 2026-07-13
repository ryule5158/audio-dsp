/**
 * @file    aud_svf.h
 * @brief   Double-sampled State Variable Filter (stable at high freq).
 *          Simultaneously outputs LP, HP, BP, Notch, Peak.
 *          Ported from DaisySP Filters/svf (MIT)
 *          Original by Andrew Simper (musicdsp.org)
 */
#ifndef AUD_SVF_H
#define AUD_SVF_H

typedef struct {
    float sr;           /* Sample rate */
    float fc;           /* Cutoff frequency (Hz) */
    float res;          /* Resonance [0, 1] */
    float drive;        /* Drive amount */
    float pre_drive;
    float freq;         /* Internal warped frequency coeff */
    float damp;         /* Damping coefficient */
    float notch, low, high, band, peak;
    float out_low, out_high, out_band, out_peak, out_notch;
    float fc_max;       /* sr / 3 */
} Aud_Svf;

void Aud_Svf_Init(Aud_Svf *svf, float sample_rate);
void Aud_Svf_Process(Aud_Svf *svf, float in);
void Aud_Svf_SetFreq(Aud_Svf *svf, float f);
void Aud_Svf_SetRes(Aud_Svf *svf, float r);
void Aud_Svf_SetDrive(Aud_Svf *svf, float d);

static inline float Aud_Svf_Low(const Aud_Svf *s)   { return s->out_low; }
static inline float Aud_Svf_High(const Aud_Svf *s)  { return s->out_high; }
static inline float Aud_Svf_Band(const Aud_Svf *s)  { return s->out_band; }
static inline float Aud_Svf_Notch(const Aud_Svf *s) { return s->out_notch; }
static inline float Aud_Svf_Peak(const Aud_Svf *s)  { return s->out_peak; }

#endif /* AUD_SVF_H */
