# MCU DSP 调用与移植速查

先打开 `project/STM32H743_Audio/MDK-ARM/STM32H743_Audio.uvprojx`，选择
`SelfTest` 目标验证工程；不要直接用 `WM8960_Stream` 验证未知接线。
四个目标的用途、参考引脚与重新生成命令见 [README](README.md)。

## 选哪一层调用

| 需求 | 入口 | 要移植的部分 |
|---|---|---|
| 单独使用滤波、振荡器、包络等 | 根目录 `aud_*.h/.c` 的 `Aud_*_Init/Process` | 算法源及其依赖，不需要 STM32 HAL |
| 使用现成的双声道效果链 | `App/Inc/audio_dsp.h` 的 `AudioDsp_*` | 效果链、根 DSP 源及 `board_config.h` |
| 驱动参考 H743 + WM8960 硬件 | `App/Inc/audio_app.h` 的 `AudioApp_*` | SAI/DMA/I2C、Codec、时钟、MPU、scatter 和板级接线 |

现有 Keil 工程已经选入 49 个 permissive DSP 源文件。若另建工程，不要把
根目录所有算法盲目打包：以该源文件组及 `THIRD_PARTY_NOTICES.md` 为准，
每个算法的依赖和许可证仍需保留。通用分析库和 LVGL 是独立可选目标。

## 效果链最小例子

把以下代码放入应用自己的 `.c` 文件，并将根目录、`App/Inc` 加入 include
路径；使用现有 `AudioDsp` 源文件组。它不初始化 Codec，也不会启动 DMA。
先调用一次 `Example_Init` 并检查返回值，再在有效音频块到达时调用
`Example_Process`。输入/输出缓冲的生命周期由调用方负责。

```c
#include "audio_dsp.h"

enum { EXAMPLE_FRAMES = 64, EXAMPLE_DELAY_SAMPLES = 4801 };
static AudioDsp effect;
static float delay_l[EXAMPLE_DELAY_SAMPLES];
static float delay_r[EXAMPLE_DELAY_SAMPLES];
static float work[EXAMPLE_FRAMES * 2];

int Example_Init(void)
{
    AudioDspParams params;
    int status = AudioDsp_Init(&effect, delay_l, delay_r,
                               EXAMPLE_DELAY_SAMPLES);
    if (status != 0) return status;
    AudioDsp_DefaultParams(&params);
    params.delay_time_ms = 100.0f;
    params.delay_mix = 0.25f;
    params.delay_feedback = 0.25f;
    AudioDsp_SetParams(&effect, &params);
    return 0;
}

int Example_Process(const int32_t *rx, int32_t *tx)
{
    return AudioDsp_ProcessPcm(&effect, work, EXAMPLE_FRAMES * 2,
                               rx, tx, EXAMPLE_FRAMES);
}
```

这里一帧是 L/R 两个采样，不是一个采样。`rx/tx` 各至少 128 个 `int32_t`，
交错排列为 L0/R0/L1/R1；有效 signed 24-bit 位于每个字的低 24 位。
这不是左对齐 24-bit，也不是每采样三个字节的 packed PCM。
工作缓冲是 CPU 浮点缓冲，不等于可直接交给 DMA 的存储区。

## 移植时不能省略的合同

- 当前 `AudioDsp_Init` 的效果链采样率固定为 **48 kHz**。只改板级宏或
  Codec 时钟不会自动改变所有算法的采样率；其他采样率需要同步修改并回归。
  单个根算法则应把实际采样率传给自己的 `Aud_*_Init`。
- `AudioDsp_Process` 接受交错 stereo float 并原地改写；`ProcessPcm` 做
  24-bit PCM 转换。两者都必须在成功初始化后调用，输入样本和参数应为有限值。
- 一个 `AudioDsp` 实例只由一个处理上下文使用。不要在 UI/中断/主循环中
  同时调用 `SetParams` 和 `Process`；在音频块边界由同一处理者应用参数。
- 延时数组必须持续有效；100 ms/48 kHz 示例每声道需要 4,801 个 float。
  初始化会设置状态和延时存储，不要每个音频块重新初始化。
- 默认效果链包含 DC blocker 和输出 soft clip；`drive=0` 只旁通过载混合，
  不表示整条链是逐位透明直通。
- 音频 DMA 继续使用 scatter/MPU 定义的 D2 SRAM 不可缓存区，不要随意改成
  栈、DTCM 或普通可缓存数组。换 MCU 时重新核对 DMA 可达性和缓存维护。
- 禁止在实时音频回调中调用 LVGL、阻塞 SPI/I2C、打印或分配内存。
  当前 LVGL 目标没有与实时音频流合并；UI 滑块不直接控制音频参数。
- MCU 外部 SDRAM 已按用户决定暂缓；当前工程没有 FMC/SDRAM 初始化。
  FPGA 不再包含 LVGL，这与 MCU 保留独立 LVGL 目标是两件事。

示例的编译检查及 MCU 构建只能验证源码与链接合同；CPU 占用、回调期限、
缓存一致性、Codec 时钟和实际音质仍需要板上测量，不能据此宣称硬件通过。
