# Explicit source manifest: adding a root aud_*.c file cannot silently change a
# release. The LGPL group is opt-in and receives a distinct archive name.
set(AUDIO_DSP_PERMISSIVE_BASENAMES
    aud_adenv.c aud_adsr.c aud_analogbassdrum.c aud_analogsnaredrum.c
    aud_autowah.c aud_chorus.c aud_clockednoise.c aud_crossfade.c
    aud_dcblock.c aud_decimator.c aud_drip.c aud_dust.c aud_flanger.c
    aud_fm2.c aud_formantosc.c aud_fractal_noise.c aud_grainlet.c
    aud_granularplayer.c aud_harmonic_osc.c aud_hihat.c aud_karplusstring.c
    aud_limiter.c aud_looper.c aud_maytrig.c aud_metro.c aud_modalvoice.c
    aud_osc.c aud_oscillatorbank.c aud_overdrive.c aud_particle.c
    aud_phaser.c aud_phasor.c aud_pitchshifter.c aud_resonator.c
    aud_samplehold.c aud_sampleratereducer.c aud_smooth_random.c aud_soap.c
    aud_stringvoice.c aud_svf.c aud_synthbassdrum.c aud_synthsnaredrum.c
    aud_tremolo.c aud_variablesawosc.c aud_variableshapeosc.c aud_vosim.c
    aud_wavefolder.c aud_whitenoise.c aud_zoscillator.c)

set(AUDIO_DSP_LGPL_BASENAMES
    aud_allpass.c aud_balance.c aud_bitcrush.c aud_blosc.c aud_comb.c
    aud_compressor.c aud_fold.c aud_jitter.c aud_line.c aud_mode.c
    aud_moogladder.c aud_nlfilt.c aud_pluck.c aud_polypluck.c aud_port.c
    aud_reverbsc.c aud_tone.c)

set(AUDIO_DSP_SOURCES "")
foreach(name IN LISTS AUDIO_DSP_PERMISSIVE_BASENAMES)
  list(APPEND AUDIO_DSP_SOURCES "${AUDIO_DSP_ROOT}/${name}")
endforeach()

set(AUDIO_DSP_LGPL_SOURCES "")
foreach(name IN LISTS AUDIO_DSP_LGPL_BASENAMES)
  list(APPEND AUDIO_DSP_LGPL_SOURCES "${AUDIO_DSP_ROOT}/${name}")
endforeach()
