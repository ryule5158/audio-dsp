# 开源音频 DSP 调研与融合决策

调研快照日期：2026-08-09。范围覆盖嵌入式 DSP、合成器、吉他/工作室效果、Linux 音频运行时、代码生成、神经网络音频和 FPGA 音频数据通路。开源项目数量没有可证明的“全部”边界，因此采用按类别穷举候选、按许可证/实时性/平台适配性择优的办法；每个采用项固定来源提交，后续升级必须重新做许可证和回归检查。

## 采用原则

1. 默认固件只直接融合 MIT、BSD、Apache-2.0 或本仓库原创实现。
2. GPL/LGPL 代码不复制进默认静态库；必须使用时建立独立构建目标并履行相应发布义务。
3. 音频回调禁止动态分配、文件 I/O、锁等待和参数结构的并发写入。
4. 先复用现有模块；只有功能、实时性或平台适配有实质收益时才替换。
5. FPGA 代码以 Vivado 2018.3 可综合的 Verilog-2001/保守 SystemVerilog 子集为基线。

## 核心候选矩阵

| 项目 | 许可证 | 候选能力 | 结论 |
|---|---|---|---|
| [CMSIS-DSP](https://github.com/ARM-software/CMSIS-DSP/tree/ec1bb75277d3e23e2fa7b8819dc734a7926b0bfb) | Apache-2.0 | Cortex-M/A FFT、FIR、IIR、矩阵和统计优化 | **采用为可选后端**；MCU 通过 Keil CMSIS-Pack/CubeMX 使用，不复制整库 |
| [DaisySP](https://github.com/electro-smith/DaisySP/tree/599511b740f8f3a9b8db72a0642aa45b8a23c3a3) | 主库 MIT；历史 DaisySP-LGPL 子集为 LGPL-2.1-only | 嵌入式合成、效果、物理建模 | **保留现有 C 端口但修正边界**；17 个 LGPL 模块默认排除，其余进入默认跨平台库 |
| [Mutable Instruments eurorack](https://github.com/pichenettes/eurorack/tree/08460a69a7e1f7a81c5a2abcc7189c9a6b7208d4) | 混合 MIT/GPL | 宏振荡器、颗粒、物理建模、随机/音序 | **保留现有固定提交端口**；GPL 模块继续显式隔离 |
| [STK](https://github.com/thestk/stk/tree/6aacd357d76250bb7da2b1ddf675651828784bbc) | MIT 风格 | 乐器物理建模、效果、控制 | **算法参考**；本地已有 pluck/modal/string，暂不引入大型 C++ 运行时 |
| [Maximilian](https://github.com/micknoise/Maximilian/tree/f937afb71aee841607f87a7f8bbd9783581fbd30) | MIT | C++ 合成、FFT、滤波和效果 | **算法/测试参考**；不作为嵌入式默认依赖 |
| [Airwindows](https://github.com/airwindows/airwindows/tree/5c8aae0c97ad547561056738a7c1d4d579113ffb) | MIT | 大量混音、饱和、空间和母带效果 | **择优参考**；只移植可验证、低状态量算法并保留通知 |
| [KORG logue SDK](https://github.com/korginc/logue-sdk/tree/c224d07adeb2a7ea04bbb9bc330d55f08305aca7) | BSD-3-Clause | 嵌入式振荡器/效果 ABI、参数模型 | **接口设计参考**；不引入 KORG 平台层 |
| [miniaudio](https://github.com/mackron/miniaudio/tree/9634bedb5b5a2ca38c1ee7108a9358a4e233f14d) | Public Domain 或 MIT-0 | 桌面/Linux 设备 I/O | **不作为 i.MX6ULL 默认层**；板上直接使用 ALSA，减少代码和线程不确定性 |
| [RTNeural](https://github.com/jatinchowdhury18/RTNeural/tree/31da15bb957942a1515f355370d90a2f1d975a5a) | BSD-3-Clause | 实时神经网络推理 | **后续可选**；i.MX6ULL A7 性能/内存实测前不进入实时默认链 |
| [NeuralAudio](https://github.com/mikeoliphant/NeuralAudio) | MIT | NAM 等神经功放模型 | **后续可选**；同样要求目标机性能证明 |
| [Faust](https://github.com/grame-cncm/faust/tree/0513a67548e4c00cbad793c6265f245132d5fcca) | 编译器 LGPL-2.1-or-later，组件另行核对 | DSP 描述生成 C/C++/LLVM | **只做离线生成工具候选**；生成物逐项审计，不把编译器链接进固件 |
| [Mozzi](https://github.com/sensorium/Mozzi/tree/7f1e5b423df8162a7e5e68a4d3dbe583712a41fb) | LGPL-2.1 | 定点合成、Arduino/MCU 音频调度 | **参考**；Arduino 调度模型和 LGPL 静态链接义务不适合默认 Keil 固件 |
| [Teensy Audio](https://github.com/PaulStoffregen/Audio) | 文件级许可证 | 定点 block graph、成熟硬件生态 | **结构参考**；硬件绑定强，导入前必须逐文件审计 |
| [Guitarix](https://github.com/brummer10/guitarix) / [KFR](https://github.com/kfrlib/kfr) | GPL 或商业双许可 | Linux 吉他效果、SIMD DSP | **不融合默认库**；许可证和 i.MX6ULL 资源不匹配 |

## FPGA 候选矩阵

| 项目 | 许可证/工具链 | 结论 |
|---|---|---|
| [gwbres/audio-dsp](https://github.com/gwbres/audio-dsp/tree/8b2c8b4293e5a1d3110e852d327c017721f46aa6) | 仓库内文件级核对；声明 Vivado 2020.1 | 架构参考：CIC/FIR/IIR、延时、频谱和 Zynq 控制；不能直接作为 2018.3 工程导入 |
| [I2S-transceiver-ZYNQ-7000-SoC](https://github.com/exarchou/I2S-transceiver-ZYNQ-7000-SoC/tree/3592bf368bae77d0360ddbc3a2cf1fbc6e443c20) | MIT | I2S 时序与仿真参考；本仓库编写独立、可参数化 RTL |
| [Audio-Processing-Artix-7](https://github.com/FallingLights/Audio-Processing-Artix-7/tree/9af90d00ccbf853027214d6399c278a0774e3625) | MIT | 延时/回声/混响结构参考；板卡与 CODEC 层不照搬 |
| [Vitis Libraries](https://github.com/Xilinx/Vitis_Libraries/tree/629b2c979f65561f07e4e87b860f306cb480895e) | Apache-2.0 | 现代 HLS 参考；不作为 Vivado 2018.3 默认依赖 |
| [opl3_fpga](https://github.com/gtaylormb/opl3_fpga/tree/491a4dc9bb25c33e0183caaf683102b9ce273bc1) | LGPL-3.0 | 完整 OPL3/FM 参考；隔离，不复制到默认 RTL |

## 融合后的功能责任

- 根目录：便携浮点 DSP 基元和模块，供 MCU 与 MPU 直接使用。
- `eurorack_algorithms/`：高复杂度合成/颗粒/物理建模；MCU 优先，MPU 按内存和许可证选择。
- `mcu/`：SAI 全双工 DMA、D-Cache 一致性、Q31/float 转换、参数快照、CMSIS-DSP 可选加速。
- `mpu/`：ALSA mmap/read-write 实时循环、预分配缓冲、线程优先级、xrun 恢复和 ARMv7-A 交叉构建。
- `fpga/`：I2S、流式定点算法、饱和/舍入、BRAM 延时、AXI-Lite 参数和 PS 示例。

## 不做的替换

- 不因“项目更新更晚”就替换已经固定提交并验证过的本地算法。
- 不把 JUCE/VST/LV2 GUI 或桌面插件框架搬到裸机目标。
- 不把深度学习模型当作 i.MX6ULL 的默认效果；必须先证明单核 A7 的最坏执行时间。
- 不把 GPL/LGPL 源码改名后混入 MIT 目录。
