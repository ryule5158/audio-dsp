# 三平台架构与硬件边界

## 总体分工

```text
控制/CV/MIDI -> 参数快照 -> DSP 图 -> 限幅/格式转换 -> 音频输出
                         |                 |
               MCU/MPU 浮点实现      FPGA Q1.23/Q1.31 实现
```

三套库共享参数语义和 48 kHz 基准，但不强迫共享同一种数值格式。MCU/MPU 以 `float` 为主；FPGA 数据面以带饱和的定点为主。所有平台都必须在音频边界做削顶保护，并把控制速率更新与逐采样处理分开。

## MCU：STM32H743IIT6 + CubeMX + Keil

推荐基准：48 kHz、双声道、每声道 24 有效位装入 32 位槽、64 帧半缓冲。H743 作 I2S/SAI 主机时，典型时钟为 LRCK=48 kHz、BCLK=3.072 MHz、MCLK=12.288 MHz。

硬件最少需要：

- 低噪声立体声 CODEC（参考 WM8960/WM8731/ADAU1761），通过 SAI/I2S 连接；控制口使用 I2C。
- CODEC 模拟电源、数字电源和耳机/线路输出按器件手册分区去耦；模拟地与数字地连续参考，不让高速回流穿过模拟输入区。
- Eurorack 音频输入先衰减/偏置/限幅到 CODEC 允许范围，输出用运放恢复电平；CV/门信号使用独立 ADC、保护和电平转换，不能直接接 MCU/音频 CODEC。
- SAI RX/TX 双 DMA 循环，缓冲区放在 DMA 可访问 SRAM；启用 D-Cache 时必须按 cache line 对齐并执行正确的 clean/invalidate。
- 电源欠压、输出静音、上电/复位顺序和看门狗；调试口、UART/USB 日志不得阻塞音频中断。

CubeMX 工程将只生成时钟、GPIO、I2C、SAI、DMA 和 cache/MPU 基础。DSP 代码位于用户区之外，避免再次生成时被覆盖；Keil 以 ArmClang、C11、`-O2/-O3` 和 Cortex-M7 FPv5 hard-float 构建。

## MPU：100ASK i.MX6ULL + Ubuntu 18.04

本机资料中的 100ASK Pro 底板原理图第 9 页确认板载 `WM8960`：控制总线为 I2C2，音频为 SAI2 的 MCLK/BCLK/SYNC/TXD/RXD，同时提供耳机、板载麦克风和差分扬声器接口。Linux 4.9.88 设备树把声卡建模为 `wm8960-audio`，codec 地址 `0x1a`，SAI2 MCLK 为 12.288 MHz。

软件路径：

```text
WM8960 <-> SAI2/ASoC <-> ALSA PCM <-> mpu/audio_engine <-> 根目录 DSP
```

默认使用 48 kHz、S32_LE 容器/24 位有效数据、双声道、小周期缓冲。音频线程预分配全部内存，可选 `SCHED_FIFO`/CPU 亲和性，参数由非实时线程写入双缓冲快照。发生 underrun/overrun 时记录计数并恢复 PCM，不在实时循环打印日志。

当前 Ubuntu 18.04 虚拟机已确认有 GCC/G++ 7.5、CMake 3.10、`arm-linux-gnueabihf-gcc` 7.5 和本机 ALSA 1.1.3 开发包。交叉构建 DSP 静态库不依赖目标 rootfs；交叉链接 ALSA 可执行文件还需要 i.MX6ULL rootfs/sysroot 中的 ARMHF `libasound`。

## FPGA：BX71 / XC7Z020 + Vivado 2018.3

BX71 用户手册确认：PL 独立 50 MHz 时钟接 XC7Z020 管脚 U18，PS 时钟为 33.333 MHz。板上没有可直接假定为音频 CODEC 的固定接口，因此默认工程只提供逻辑与约束模板，最终 XDC 必须按实际外接音频前端确定。

推荐硬件：

- 3.3 V I/O 的 I2S ADC/DAC 或完整 CODEC 板，至少引出 MCLK、BCLK、LRCK、ADC_SD、DAC_SD；需要寄存器配置的器件另接 I2C/SPI。
- 48 kHz/24-bit/stereo 时，生成 12.288 MHz MCLK、3.072 MHz BCLK 和 48 kHz LRCK；时钟由一个 clocking wizard/MMCM 域产生，避免用普通逻辑随意分频后跨域。
- GPIO1 Bank 电压可调；接线前先测实物 Bank 电压并在 XDC 选一致的 IOSTANDARD。所有外部音频数字线与 FPGA 共地，模拟输入输出仍需 CODEC 前端和保护。
- 若用于 Eurorack，模拟电平适配与 MCU 方案相同；FPGA 引脚绝不能直接承受双极性音频/CV。

PL 数据面采用带 `valid/ready` 的有符号样本流：I2S RX -> DC blocker/增益 -> 可选合成/效果 -> 饱和 -> I2S TX。PS 通过 AXI-Lite 更新影子参数，参数只在帧边界生效。大延时使用 BRAM；长混响/卷积优先放 DDR + AXI DMA，并明确跨时钟 FIFO。

Vivado 2018.3 的默认验证链为：RTL lint/语法 -> xsim 自检 testbench -> 综合 -> 实现/时序 -> 生成 bitstream。完成这些仍不等于 CODEC 接线和模拟音质已经通过。

## 验收指标

| 类别 | 最低证据 |
|---|---|
| 功能 | 直通、静音、1 kHz 正弦、每个效果旁路/启用、参数极值 |
| 数值 | 无 NaN/Inf，定点无环绕，削顶行为与文档一致 |
| 实时 | MCU 最坏 callback 占用；MPU xrun 计数；FPGA 全路径时序裕量 |
| 音频 | 频响、THD+N、底噪、串扰、削顶电平、端到端延迟 |
| 稳定性 | 典型温度/电源下连续运行至少 8 小时，无 xrun、死锁或状态漂移 |
