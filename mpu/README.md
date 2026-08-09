# MPU library - 100ASK i.MX6ULL

目标环境：VMware Ubuntu 18.04，ARMHF 交叉编译，目标 Linux 通过 ALSA/ASoC 驱动板载 WM8960。

该目录将提供可独立交叉编译的 DSP 静态库、ALSA 双工引擎、xrun 统计、实时线程配置和目标板部署脚本。根目录纯 C 模块是默认算法集合；大型 Eurorack C++ 模块按内存与许可证单独启用。

当前阶段：本机原理图、设备树与虚拟机工具链已核对，代码和 VM 构建验证将在后续阶段加入。
