# Third-party notices and integration policy

The repository contains or derives from third-party projects. The authoritative license text shipped with each imported component remains controlling.

## Present in the repository

- DaisySP-derived C modules: Electrosmith and upstream authors, mixed MIT and LGPL-2.1-only. The 17 DaisySP-LGPL-derived module pairs are enumerated in the root `LICENSE`, carry file-level SPDX identifiers, and are excluded from default MCU/MPU builds. See `LICENSES/MIT.txt` and `LICENSES/LGPL-2.1-only.txt`.
- Mutable Instruments Eurorack algorithms pinned to `08460a69a7e1f7a81c5a2abcc7189c9a6b7208d4`: mixed MIT and GPL-3.0-or-later. See `eurorack_algorithms/LICENSE`, `LICENSES/MIT.txt`, `LICENSES/GPL-3.0-or-later.txt`, and `eurorack_algorithms/module_manifest.json`.
- STM32CubeH7 1.13.0 files generated into `mcu/project/STM32H743_Audio/Drivers`: CMSIS core is Apache-2.0; the STM32H7 device component identifies Apache-2.0 as its fallback license; the HAL component identifies BSD-3-Clause as its fallback license. The controlling license files are retained beside those components.
- LVGL v9.5.0, commit `85aa60d18b3d5e5588d7b247abf90198f07c8a63`: official manually vendored source, not a Keil Pack. See `mcu/project/STM32H743_Audio/App/LVGL/ORIGIN.md`, `LICENCE.txt` (MIT), `COPYRIGHTS.md` and retained component notices. Only the optional LVGL target compiles it; project-local configuration/ports are identified separately.

- User-provided Generic_DSP H743 DSP integration at mcu/project/STM32H743_Audio/App/Generic_DSP: copied byte-for-byte from review commit 416270a254795843b6f1854074380015637d1f95. The upstream DSP tree supplied no license metadata in the reviewed commit; see App/Generic_DSP/ORIGIN.md. No license is asserted for those files.

## Optional external dependencies, not vendored

- ARM CMSIS-DSP: Apache-2.0. Obtain through the Keil CMSIS-Pack/Cube ecosystem or upstream.
- ALSA userspace library: LGPL-2.1-or-later. The MPU application links the target system shared library; source is not vendored here. This is distinct from the LGPL-2.1-only DaisySP-derived source modules above.
- Xilinx Vivado/SDK generated support: governed by AMD/Xilinx tool and generated-file licenses; no generated proprietary IP output is committed by default.

## Reference-only projects

Projects listed as reference-only in `docs/OPEN_SOURCE_SURVEY.md` have not been copied into this repository. Algorithmic ideas are reimplemented independently or left as future work. Moving any of them into a default build requires a fresh file-level license review and an update to this notice.
