# CMSIS-DSP dependency contract

The Generic DSP target uses the installed ARM CMSIS-DSP Pack rather than a
second vendored copy:

- CMSIS-DSP component: CMSIS:DSP:Source version 1.16.2
- CMSIS core component: CMSIS:CORE version 6.2.0
- Pack versions selected by the Keil project: ARM/CMSIS-DSP/1.16.2 and
  ARM/CMSIS/6.3.0
- Required target: STM32H743_Audio_Generic_DSP

The project metadata scopes both components to that target. The current Generic_DSP
sources use arm_cfft_f32, the predefined 64/128/256/512/1024/2048/4096
complex FFT instances, arm_cmplx_mag_f32, arm_fir_f32,
arm_fir_init_f32, and arm_max_f32. Keil's RTE resolver supplies the Pack
include and source closure; a successful build must still be checked after
CubeMX regeneration. The pack's Apache-2.0 license applies to the Pack, not to
the Generic_DSP sources documented in ORIGIN.md.
