# MCU library - STM32H743IIT6

目标工具链：STM32CubeMX + Keil MDK 5/ArmClang。该目录提供 SAI/I2S 全双工 DMA 适配、32-bit PCM 与 float 转换、D-Cache 安全缓冲、无堆分配参考效果链和可选 CMSIS-DSP 集成点。

根目录 DSP 模块保持 HAL 无关；只有本目录允许包含 STM32 HAL 头文件。任何 CubeMX 生成工程都必须保留用户区，并把音频缓冲放在 DMA 可见内存。

## 目录

- `include/` + `src/`：HAL 无关 PCM bridge 与 synth/effect chain。
- `ports/stm32h743_hal/`：CubeMX HAL SAI 双缓冲 DMA 与 cache 维护。
- `examples/`：复制到 CubeMX USER CODE 区的初始化/回调模板。
- `cubemx/STM32H743IIT6_AUDIO_SETUP.md`：逐项配置和硬件时钟检查。
- `keil/build_armclang.ps1`：使用本机 Keil ArmClang 构建静态库。
- `tests/`：PCM 边界、削顶和 block 计数测试。

默认参考链为：输入增益/可混合振荡器 -> DC blocker -> SVF -> overdrive -> feedback delay -> soft clip。参数只在 block 边界通过 `AudMcuFxChain_SetParams()` 更新；调用者必须把 UI/CV/MIDI 值先复制成完整快照。
