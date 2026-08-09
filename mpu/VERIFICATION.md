# i.MX6ULL / Ubuntu 18.04 verification

Verified on 2026-08-09 in the configured VMware guest:

- guest identity: user `hry`, host `ubuntu`, Ubuntu 18.04.6 LTS;
- native compiler: GCC 7.5.0, CMake 3.10.2, ALSA development package 1.1.3;
- host Release build: portable permissive DSP archive, MPU core, ALSA runtime and `audio_dsp_imx6ull` all built;
- host tests: `MPU_PCM_TEST_OK` and `MPU_FX_TEST_OK`, 2/2 CTest cases passed;
- cross compiler: `arm-linux-gnueabihf-gcc` 7.5;
- cross Release build: `libaudio_dsp_portable.a` and `libaud_mpu_core.a` built for ARMv7-A hard-float with NEON/VFPv4;
- `arm-linux-gnueabihf-objdump` identifies the MPU core objects as `elf32-littlearm`, architecture `arm`.
- explicit `AUD_DSP_ENABLE_LGPL=ON` host build produced the separately named `libaudio_dsp_portable_lgpl.a`; the default archive remained permissive-only.

The native ALSA executable was linked but not connected to a physical WM8960 in the VM. The ARM application was not cross-linked because the VM does not yet have an ARMHF i.MX6ULL sysroot containing `libasound`. No image was copied to or run on the physical i.MX6ULL board, so codec routing, real-time scheduling permissions, xrun performance, latency and analog audio quality remain hardware acceptance items.

Reproduce the verified build from the repository root:

```sh
sh mpu/scripts/build_vm.sh
```
