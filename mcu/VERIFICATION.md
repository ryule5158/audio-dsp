# MCU verification record

Date: 2026-08-09

## Static library build

- Tool: Keil MDK Plus 5.43, Arm Compiler for Embedded 6.24 (`armclang`/`armar`).
- Target flags: Cortex-M7, FPv5-D16, hard-float, C11, `-O2`, `-Wall -Wextra -Werror`.
- Result: 66 root `aud_*.c` sources plus 2 MCU sources compiled into 68-object `audio_dsp_h743.lib`.
- HAL adapter and CubeMX callback example separately compiled with the STM32Cube H7 v1.13.0 H743 HAL/CMSIS headers under the same warning policy.

Reproduce on this workstation:

```powershell
& .\mcu\keil\build_armclang.ps1
```

Expected terminal markers:

```text
MCU_ARMCLANG_BUILD_OK=...\audio_dsp_h743.lib
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
```

## Not yet proven

- No H743 board was flashed in this stage.
- SAI MCLK/BCLK/LRCK, CODEC register setup, DMA callback budget and analog audio quality have not been measured.
- The example linker sections `.AudioDmaBuffer` and `.AudioDspState` still need mapping to the chosen board's SRAM layout and confirmation in the Keil map file.
