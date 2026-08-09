# AUDIO_DSP 多平台音频 DSP 库

本仓库面向 DIY 模块合成器与数字效果器。已有根目录 C 代码提供振荡器、包络、滤波器、动态、调制、失真、延时/混响、粒子/颗粒、鼓合成和物理建模；`eurorack_algorithms/` 提供 Mutable Instruments 算法的 HAL 无关 C API。

本轮扩展把硬件相关部分明确分成三个目标：

- [`mcu/`](mcu/README.md)：STM32H743IIT6，STM32CubeMX + Keil MDK/ArmClang。
- [`mpu/`](mpu/README.md)：100ASK i.MX6ULL，Ubuntu 18.04 交叉构建，Linux ALSA + 板载 WM8960。
- [`fpga/`](fpga/README.md)：BX71 / XC7Z020，Vivado 2018.3，PS 控制 + PL 定点流处理。

调研、许可证和平台分工见：

- [`docs/OPEN_SOURCE_SURVEY.md`](docs/OPEN_SOURCE_SURVEY.md)
- [`docs/PLATFORM_ARCHITECTURE.md`](docs/PLATFORM_ARCHITECTURE.md)
- [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md)

## 交付边界

软件构建、仿真和静态检查只能证明源码与工具链相容。接上真实 ADC/DAC、音频 CODEC、Eurorack 电平接口后，还必须测量采样时钟、无信号噪声、削顶、端到端延迟、CPU/FPGA 裕量和长时间稳定性，才能称为硬件验收通过。
