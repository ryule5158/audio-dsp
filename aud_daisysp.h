/**
 * @file    aud_daisysp.h
 * @brief   Audio DSP Library — ported from Electrosmith DaisySP (MIT)
 * @note    All original copyrights belong to Electrosmith Corp and original
 *          authors (Emilie Gillet, Andrew Simper, Stephen Hensley, etc.)
 *
 *          Translated from C++ to C for STM32H7 bare-metal projects.
 *          Skip aud_fir (use CMSIS arm_fir_f32 directly) and
 *          aud_biquad (use DSP/Filter.h IIR_Design / IIR_Cascade).
 */
#ifndef AUD_DAISYSP_H
#define AUD_DAISYSP_H

/* ===== Core Utilities ===== */
#include "aud_dsp.h"

/* ===== Control ===== */
#include "aud_phasor.h"
#include "aud_adenv.h"
#include "aud_adsr.h"
#include "aud_line.h"

/* ===== Filters ===== */
#include "aud_svf.h"
#include "aud_onepole.h"
#include "aud_tone.h"
#include "aud_allpass.h"
#include "aud_comb.h"
#include "aud_mode.h"
#include "aud_moogladder.h"
#include "aud_nlfilt.h"
#include "aud_soap.h"

/* ===== Dynamics ===== */
#include "aud_limiter.h"
#include "aud_crossfade.h"
#include "aud_compressor.h"
#include "aud_balance.h"

/* ===== Synthesis ===== */
#include "aud_osc.h"
#include "aud_oscillatorbank.h"
#include "aud_fm2.h"
#include "aud_formantosc.h"
#include "aud_variablesawosc.h"
#include "aud_variableshapeosc.h"
#include "aud_vosim.h"
#include "aud_zoscillator.h"
#include "aud_harmonic_osc.h"
#include "aud_blosc.h"

/* ===== Effects ===== */
#include "aud_chorus.h"
#include "aud_flanger.h"
#include "aud_phaser.h"
#include "aud_tremolo.h"
#include "aud_overdrive.h"
#include "aud_wavefolder.h"
#include "aud_autowah.h"
#include "aud_pitchshifter.h"
#include "aud_decimator.h"
#include "aud_sampleratereducer.h"
#include "aud_bitcrush.h"
#include "aud_fold.h"
#include "aud_reverbsc.h"

/* ===== Noise ===== */
#include "aud_whitenoise.h"
#include "aud_clockednoise.h"
#include "aud_dust.h"
#include "aud_fractal_noise.h"
#include "aud_grainlet.h"
#include "aud_particle.h"
#include "aud_smooth_random.h"
#include "aud_samplehold.h"

/* ===== Physical Modeling ===== */
#include "aud_karplusstring.h"
#include "aud_polypluck.h"
#include "aud_pluck.h"
#include "aud_resonator.h"
#include "aud_modalvoice.h"
#include "aud_stringvoice.h"
#include "aud_drip.h"

/* ===== Drums ===== */
#include "aud_analogbassdrum.h"
#include "aud_analogsnaredrum.h"
#include "aud_synthbassdrum.h"
#include "aud_synthsnaredrum.h"
#include "aud_hihat.h"

/* ===== Sampling ===== */
#include "aud_granularplayer.h"

/* ===== Utility ===== */
#include "aud_dcblock.h"
#include "aud_delayline.h"
#include "aud_metro.h"
#include "aud_port.h"
#include "aud_jitter.h"
#include "aud_looper.h"
#include "aud_maytrig.h"

#endif /* AUD_DAISYSP_H */
