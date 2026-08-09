# Third-party notices and integration policy

The repository contains or derives from third-party projects. The authoritative license text shipped with each imported component remains controlling.

## Present in the repository

- DaisySP-derived C modules: Electrosmith and upstream authors, MIT. See the root `LICENSE` and source headers.
- Mutable Instruments Eurorack algorithms pinned to `08460a69a7e1f7a81c5a2abcc7189c9a6b7208d4`: mixed MIT and GPL-3.0-or-later. See `eurorack_algorithms/LICENSE`, `LICENSES/MIT.txt`, `LICENSES/GPL-3.0-or-later.txt`, and `eurorack_algorithms/module_manifest.json`.

## Optional external dependencies, not vendored

- ARM CMSIS-DSP: Apache-2.0. Obtain through the Keil CMSIS-Pack/Cube ecosystem or upstream.
- ALSA userspace library: LGPL-2.1-or-later. The MPU application links the target system library; source is not vendored here.
- Xilinx Vivado/SDK generated support: governed by AMD/Xilinx tool and generated-file licenses; no generated proprietary IP output is committed by default.

## Reference-only projects

Projects listed as reference-only in `docs/OPEN_SOURCE_SURVEY.md` have not been copied into this repository. Algorithmic ideas are reimplemented independently or left as future work. Moving any of them into a default build requires a fresh file-level license review and an update to this notice.
