# MCU verification record

Date: 2026-08-09

## Static library build

- Tool: Keil MDK Plus 5.43, Arm Compiler for Embedded 6.24 (`armclang`/`armar`).
- Target flags: Cortex-M7, FPv5-D16, hard-float, C11, `-O2`, `-Wall -Wextra -Werror`.
- Default result: 49 permissive root `aud_*.c` sources plus 2 MCU sources compiled into the 51-object `audio_dsp_h743.lib`; all 17 DaisySP-LGPL-derived objects were excluded.
- Explicit compatibility result: `-IncludeLgpl` compiled all 66 root sources plus 2 MCU sources into the separately named 68-object `audio_dsp_h743_lgpl.lib`.
- HAL adapter and CubeMX callback example separately compiled with the STM32Cube H7 v1.13.0 H743 HAL/CMSIS headers under the same warning policy.

Reproduce on this workstation:

```powershell
& .\mcu\keil\build_armclang.ps1
```

Expected terminal markers:

```text
MCU_ARMCLANG_BUILD_OK=...\audio_dsp_h743.lib
MCU_LGPL_MODULES_INCLUDED=0
MCU_HAL_PORT_ARMCLANG_OK=1
```

## Executed unit tests

The tests were executed with GCC 7.5 on the verified Ubuntu 18.04 VMware guest:

- `test_mcu_audio`: Q1.31 conversion, S24-in-S32 alignment, positive/negative saturation, NaN zeroing, block accounting and over-size rejection.
- `test_root_regressions`: water-drop resonators 1/2 receive their calculated inputs; pitch read phase changes with a nonzero semitone shift and the public shift range clamps to +/-24 semitones.

Observed markers:

```text
MCU_PCM_TEST_OK
MCU_ROOT_REGRESSIONS_OK
MCU_LGPL_OPTIONAL_TEST_OK
```

The optional ReverbSc test ran under GCC 7.5 AddressSanitizer/UndefinedBehaviorSanitizer and verifies that all eight delay-line ranges remain inside `aux[]`, do not overlap, and produce finite output. This covers the corrected sample-count versus byte-count allocation bug. It does not change the fact that this module is excluded from the default library.

## Not yet proven

- No H743 board was flashed in this stage.
- SAI MCLK/BCLK/LRCK, CODEC register setup, DMA callback budget and analog audio quality have not been measured.
- The example linker sections `.AudioDmaBuffer` and `.AudioDspState` still need mapping to the chosen board's SRAM layout and confirmation in the Keil map file.
