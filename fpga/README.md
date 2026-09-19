# BX71 Zynq-7020 audio DSP — Vivado 2018.3 / XSDK

This is a source/build-verified reference, not a physically validated board
release. Use the `xc7z020clg400-2` board/part contract only after checking the
actual BX71 hardware. Do not download the images to an unverified board.

## Native projects and reproducible build

- [Self-test project](projects/audio_dsp_bx71_selftest/audio_dsp_bx71_selftest.xpr)
- [PS + audio project](projects/audio_dsp_bx71_soc_i2s/audio_dsp_bx71_soc_i2s.xpr)
- [PS block design](projects/audio_dsp_bx71_soc_i2s/audio_dsp_bx71_soc_i2s.srcs/sources_1/bd/audio_soc/audio_soc.bd)

The XPR roots and RTL references are relative. Source BD/wrapper files are
included; Vivado-generated licensed IP products, caches and runs are not.
Vivado 2018.3 regenerates those products from its installed IP catalog. The
`check_native_project.tcl` command opens the actual XPR, generates BD products,
and rejects missing/out-of-project inputs. The native projects are GUI entry
points; the release-check build uses a separate, generated `fpga/build` tree.

From the repository root, in PowerShell:

```powershell
& .\fpga\open_project.ps1 -Profile selftest
& .\fpga\open_project.ps1 -Profile soc_i2s
& .\fpga\build.ps1 -Profile selftest -Action all
& .\fpga\build.ps1 -Profile soc_i2s -Action all
& .\fpga\build_software.ps1 -Profile audio
```

Default tools are `E:\Vivado\2018.3\bin\vivado.bat` and
`E:\SDK\2018.3\bin\xsct.bat`; both wrappers accept explicit tool locations.
`vivado/materialize_native_projects.tcl` recreates the two native projects.
It replaces their generated project configuration: preserve manual GUI edits
before explicitly running it.

The independent `wm8960_waveshare` profile provides a PL codec-master demo.
`i2s_external` has intentionally unassigned external I/O and blocks bitstream
delivery until a board-specific constraint profile is supplied.

## DSP 调用与移植合同

音频格式为 stereo、24-bit signed Q1.23、每声道 32 BCLK；参考采样率
48 kHz，Codec 提供 3.072 MHz BCLK。数据链在 BCLK 域执行：

```text
I2S RX -> DC blocker -> Q3.23 gain -> NCO additive mix
       -> saturation -> stereo BRAM feedback delay -> I2S TX
```

默认每通道 16,384 个 BRAM 存储位置，可设置延时 `1..16383` 个采样
（48 kHz 下最大约 341.3 ms）；输入 0 会按 1 处理，不表示零延时。
需要直通时设置 `DELAY_MIX=0`。延时核只在 `ready=1` 时接受输入，
一次处理占三个时钟；单独复用此核时必须保留 ready/valid 合同。
复位通过历史计数屏蔽未写过的 BRAM，不依赖全 RAM 清零。

PS 调用入口是 [aud_fpga_regs.h](software/baremetal/src/aud_fpga_regs.h)，
完整初始化例程在 [main.c](software/baremetal/src/main.c)。参考 BD 的
AXI 基地址为 `0x43C00000`，移植时以实际 HDF/`xparameters.h` 为准。
`AudFpga_PhaseIncrement(hz, fs)` 使用整数 Hz，返回
`floor(hz * 2^32 / fs)`；应用应限制到有效采样带宽，不能当作抗混叠振荡器。

| 偏移 | 寄存器 | 调用合同 |
|---|---|---|
| `0x00` | CONTROL | 写 bit0=1 提交；读 bit0=update toggle、bit1=ack、bit2=pending |
| `0x04` | GAIN | signed 26-bit Q3.23；`0x00800000` 为 1.0 |
| `0x08` | DC_R | signed Q1.23；示例约 0.995，应用限制稳定范围 |
| `0x0C` | PHASE_INC | 32-bit NCO 相位步进 |
| `0x10` | OSC_SELECT | 0 saw / 1 triangle / 2 square / 3 noise |
| `0x14` | SYNTH_LEVEL | signed Q1.23；0 关闭附加合成音 |
| `0x18` | DELAY_SAMPLES | `1..16383`；0 被限制为 1 |
| `0x1C` | FEEDBACK | signed Q1.23；应用应限幅，例如绝对值不超过 0.95 |
| `0x20` | DELAY_MIX | 0=dry、`0x7FFFFF`=wet，中间为线性混合 |
| `0x24` | ID | 只读 `0x41554431`（AUD1） |

控制端按“读 ID 和 pending → 写完一组参数 → COMMIT → 有界等待 pending
清零”调用，且只能有一个参数写入者。pending 期间写操作返回 SLVERR，
不能连续盲写提交；新参数只在音频帧边界生效。音频域复位参数为全零，
至少需要一次成功 COMMIT 才解除该静音状态。没有 BCLK/LRCLK 时不会得到
音频域 ACK；超时应报告故障，不应绕过握手或改写参数总线。

移植保留定点格式、帧边界 CDC 和时钟/复位合同；仅换 Codec 时先核对其
主从模式、I2S 时隙、控制协议、MCLK 与模拟路径，再选择或新建 XDC。
Eurorack 双极性音频/CV 不可直连 FPGA，必须经适配与保护电路。

## Verified evidence (not hardware measurements)

The 2026-09-11 all-flow logs passed the self-checking RTL simulation and
produced these routed artifacts:

| Profile | WNS / WHS (ns) | LUT / FF | RAMB36 / DSP48 | Output |
|---|---:|---:|---:|---|
| selftest | 2.038 / 0.130 | 1280 / 512 | 24 / 22 | 4,045,681-byte bitstream |
| soc_i2s | 38.658 / 0.121 | 1759 / 1281 | 24 / 22 | 4,045,677-byte bitstream; 19,805-byte HDF |

Both have zero RAM64M cells and **49 DRC warnings each**, not zero DRCs.
The common warnings are DPIP-1 (18), DPOP-1 (15) and DPOP-2 (15), concerning
DSP pipelining. Selftest adds ZPS7-1 (PS7 block required); its PL-only image
does not establish PS boot/init readiness. Soc_i2s adds IOSR-1 (I/O reset
sharing). The timing figures apply to the declared clocks/exceptions, including
the selftest's 1000-cycle, sample-enable-qualified DSP multicycle constraint;
they are not unconstrained single-cycle DSP throughput measurements.

Full reports: `build/reports/<profile>/`. Verified output files:
`build/output/audio_dsp_bx71_selftest.bit`,
`audio_dsp_bx71_soc_i2s.bit`, `audio_dsp_bx71_soc_i2s.hdf`.
Exact hashes and earlier failed attempts are in `docs/PROJECT_WORKLOG.md`,
entries W-119 and W-120. Reports and binaries are build artifacts, not Git input.

The RTL testbench also checks delay-pointer wrap, history invalidation after
reset, zero-delay clamping, feedback decay, dry bypass, frame-boundary parameter
application, AXI byte strobes and rejected invalid writes. Run the bounded
self-check without redoing implementation:

```powershell
& .\fpga\build.ps1 -Profile selftest -Action sim
```

On 2026-09-19 this extended test passed with actual process exit 0 and
`FPGA_RTL_TESTS_OK` at 4,720 ns. Both native projects also passed a fresh relocated
copy check containing only four native input files plus source/scripts:
selftest 14 resolved files, soc_i2s 89 after IP regeneration and BD validation.
This was a source-portability check, not another place/route or board run.

## PS audio software

The generated XSDK workspace is `fpga/build/xsdk`, with hardware `audio_hw`,
standalone BSP `audio_bsp` and application `wm8960_demo`; open that workspace in
XSDK 2018.3 after running the software wrapper. PS DDR is deliberately disabled.
The script corrects the old SDK's default DDR linker mappings to low OCM and
validates the ELF's actual entry and every LOAD segment against `0..0x30000`.

On 2026-09-12 the audio software build completed with process exit 0 and the
success marker. Its 284,932-byte ELF has one LOAD segment `0..0xD840`, entry 0.
ELF file size includes debug information and is not its memory footprint.
Hsi 61-9 and the initial managed-builder pre-clean diagnostic are retained in
the log; the later compile/link/direct make and ELF range checks succeeded.
The program was **not executed** on a PS and no codec/AXI activity was measured.

After withdrawing PS LVGL, the audio-only rebuild on 2026-09-14 again exited 0
and produced the identical ELF SHA-256 recorded in W-120. Its application has
only `main.o` and `wm8960.o`; it has no LVGL source dependency. On 2026-09-19
readelf and SHA-256 were rechecked, and the removed `-Profile lvgl` option was
confirmed to fail before touching the audio output.

## Scope: audio DSP only

Per the 2026-09-14 scope decision, FPGA/PS LVGL is not part of this delivery.
Its minimal headless experiment left only 4,624 bytes of low OCM; the source
port and build option have been withdrawn. The audio build does not import
MCU LVGL or depend on it. Ignored historical experiment logs/workspaces are
not supported delivery targets. MCU LVGL remains a separate MCU requirement.

## Physical acceptance still required

Confirm the actual board revision, FPGA speed grade, bank voltage, reset and
PS boot configuration, then measure the named clocks and I/O. For WM8960 verify
the module schematic and 3.3 V interface, external MCLK, codec-master BCLK/LRCLK,
I2C ACKs, ADC/DAC data and analog input/output. A bitstream, HDF, ELF, passing
testbench or ready UART message cannot replace those measurements.
