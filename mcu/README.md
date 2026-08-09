# MCU library - STM32H743IIT6

目标工具链：STM32CubeMX + Keil MDK 5/ArmClang。该目录将提供 SAI/I2S 全双工 DMA 适配、32-bit PCM 与 float 转换、参数快照、D-Cache 安全缓冲和可选 CMSIS-DSP 后端。

根目录 DSP 模块保持 HAL 无关；只有本目录允许包含 STM32 HAL 头文件。任何 CubeMX 生成工程都必须保留用户区，并把音频缓冲放在 DMA 可见内存。

当前阶段：平台与硬件基线已确认，代码和 CubeMX/Keil 集成工程将在下一阶段加入。
