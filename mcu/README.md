# STM32H743IIT6 音频开发模板

这是可由 STM32CubeMX 重新生成、可由 Keil MDK 直接打开并已用 ArmClang
实际编译的完整工程，不再需要把零散示例复制进新项目。

工程入口：

- CubeMX：`project/STM32H743_Audio/STM32H743_Audio.ioc`
- Keil：`project/STM32H743_Audio/MDK-ARM/STM32H743_Audio.uvprojx`
- 板级合同：`project/STM32H743_Audio/App/Inc/board_config.h`
- 音频应用与效果链：`project/STM32H743_Audio/App/`
- 可复现脚本：`scripts/`
- 根目录 DSP 源文件组：Keil 工程中的 `Library/DSP permissive`（49 个 `.c`，
  与 `mpu/cmake/portable_sources.cmake` 同一份许可白名单）。

## 固定硬件合同

本模板规定的参考硬件是 **STM32H743IIT6（LQFP176）+ 外接 WM8960
模块**。下表是模板要求的接线，不代表用户现有底板已经这样连接。

| STM32H743 引脚 | 方向 | WM8960 信号 | 作用 |
|---|---:|---|---|
| PE6 / SAI1_SD_A | 输出 | DACDAT | DAC 串行数据 |
| PG7 / SAI1_MCLK_A | 输出 | MCLK | 12.288 MHz 主时钟 |
| PE5 / SAI1_SCK_A | 输出 | BCLK | I2S 位时钟 |
| PE4 / SAI1_FS_A | 输出 | LRCLK | 48 kHz 左右声道时钟 |
| PE3 / SAI1_SD_B | 输入 | ADCDAT | ADC 串行数据 |
| PB8 / I2C1_SCL | 输出 | SCL | Codec 控制时钟 |
| PB9 / I2C1_SDA | 双向 | SDA | Codec 控制数据 |
| GND | — | GND | 必须共地 |

I2C SCL/SDA 需要接到 3.3 V 的外部上拉，常用 2.2–4.7 kΩ；若模块已经
带上拉，不要重复并联到过低阻值。控制地址固定为 7-bit `0x1A`。当前
驱动启用 WM8960 的 `LINPUT1/RINPUT1`、ADC、DAC 和 `LOUT1/ROUT1` 路径；
只引出扬声器端子的模块需要另配 class-D 输出寄存器，不能直接视为已支持。

H743 最小系统仍需按数据手册完成全部 VDD/VDDA/VREF、去耦、VCAP1/VCAP2、
NRST、BOOT 和 SWD。固件使用内部 HSI 作为 PLL 源，因此功能启动不依赖
外部晶振；若用于录音、乐器或低抖动成品，应在 PCB 上提供合适的外部
HSE/有源时钟，并在 `.ioc` 中重新求解 PLL1/PLL3 后再验证 12.288 MHz MCLK。
不要假定任意 WM8960 小板可以直接接 5 V；电源和模拟输入幅度必须以该
模块原理图为准。

## 已配置的软件路径

- STM32H743IIT6，400 MHz Cortex-M7；AXI/HCLK 200 MHz。
- SAI1 Block A：I2S master TX；Block B：内部同步 slave RX。
- 48 kHz、stereo、24-bit，PLL3 `M=40, N=192, P=25`，MCLK 12.288 MHz；
  I2S 的 64-bit stereo frame 产生 3.072 MHz BCLK。五个 SAI AF 引脚均配置为
  `GPIO_SPEED_FREQ_VERY_HIGH`，避免 12.288 MHz MCLK 被低速 GPIO 边沿限制。
- DMA2 Stream0 TX / Stream1 RX，word 对齐、circular、very-high priority。
- 64-frame 半缓冲；RX half/full callback 处理对应 ping-pong 区域。
- I-Cache 与 D-Cache 开启；`s_rx/s_tx` 位于 D2 SRAM
  `0x30000000..0x300007FF`，所在 32 KiB MPU 区域不可缓存。
- WM8960 最小写寄存器驱动：软件复位、VMID/VREF、I2S slave 24-bit、
  立体声输入/ADC/DAC/耳机输出、启动静音与 DMA 就绪后解除静音。
- 无堆分配效果链直接调用根库 `aud_dcblock`、`aud_svf`、`aud_overdrive`、
  `aud_delayline`、`aud_osc`：DC blocker、可混合低通、soft overdrive、
  可调且默认精确 100 ms 的 stereo feedback delay、输出限幅。`drive=0` 是
  真正 dry bypass，不会因 overdrive 内部零前置增益把默认音频静音。
- SAI error callback 只登记故障；主循环执行 Codec 静音、停止双向 DMA、清空
  DMA/效果器状态、依次启动 RX/TX，全部成功后才解除静音。任一步失败都会
  停止两路 DMA 并保持静音，不会留下半启动流。

WM8960 的 Audio Interface 2（R9）设置 `ALRCGPIO=1`，因此单根 PE4/DACLRC
同时作为 ADC frame clock；如果你的 Codec 板把 ADCLRC 单独引出，应按板级
接线改写该寄存器与 SAI 合同，不能只改注释。

`AUDIO_BOARD_ENABLE_WM8960_STREAM` 默认为 `0`。默认固件只初始化外设并
运行确定性的 DSP 自检代码，不访问外接 Codec，也不启动 SAI DMA。完成
接线、电源和模拟电平检查后，在 Keil 目标下拉框选择
`STM32H743_Audio_WM8960_Stream` 才会启用硬件流；不需要手改生成的头文件。
调试器可观察：

- `g_audio_app_status == 0`：初始化路径成功；
- `g_audio_app_status == -2`：DSP 启动自检失败；
- `g_audio_app_status == -3`：WM8960 I2C 初始化/静音控制失败；
- `g_audio_app_status == -4`：SAI DMA 启动或运行错误；
- `g_audio_callback_errors`：DMA callback 累计错误数。

WM8960 控制口是 write-only，无法读回寄存器；硬件验收需要同时测量 I2C
ACK、MCLK/BCLK/LRCLK、ADCDAT/DACDAT 以及实际模拟输入输出。

## 重新生成与编译

本机固定验证环境：STM32CubeMX 6.17.0、STM32CubeH7 1.13.0、Keil MDK
5.43.1、ArmClang 6.24、STM32H7xx DFP 4.1.3。

从仓库根目录执行完整验收（两个 Keil target 都会构建）：

```powershell
& .\mcu\scripts\verify_all.ps1
```

也可以分开执行：

```powershell
& .\mcu\scripts\regenerate_cubemx.ps1
& .\mcu\scripts\build_keil.ps1
# 只构建某一个变体时：
& .\mcu\scripts\build_keil.ps1 -Target SelfTest
& .\mcu\scripts\build_keil.ps1 -Target WM8960
```

CubeMX 会覆盖 IDE 工程配置，因此 `regenerate_cubemx.ps1` 在生成后调用
`postgenerate_keil.ps1`，恢复 ArmClang、App 源文件组、头文件路径和自定义
scatter 文件，并同步 `.uvprojx/.uvoptx` 的两个 target。再生成脚本只接受
本轮刷新的关键文件或本轮新鲜完成日志；预存的 `main.c/sai.c` 不能冒充
本次 CubeMX 成功。不要只运行裸 CubeMX CLI 后立刻编译。

## 2026-08-24 验证证据

最终从 `.ioc` 重新生成并在干净输出目录全量编译：

```text
CubeMX contract: no IP-not-ready, invalid-value or parameter-conflict blocker
Compiler: ArmClang V6.24
Program Size: SelfTest Code=25408, ZI=41012;
              WM8960 Stream Code=32608, ZI=41540
两个 `.axf` 均为 0 Error(s), 0 Warning(s)，每个 target 编译 49 个根 DSP 对象
map: s_rx=0x30000000, s_tx=0x30000400, RW_AUDIO_DMA size=0x800
```

原始运行日志保存在忽略版本控制的 `mcu/build/`，可由上述脚本重建。这个
结论证明 CubeMX 生成、Keil 编译和链接布局成立；尚未代表固件已烧录，也
未代表 WM8960、电源、时钟、模拟链路或真实音频已经通过物理测量。
