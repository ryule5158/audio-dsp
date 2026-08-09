# FPGA/SoC library - BX71 XC7Z020

目标工具链：Vivado 2018.3。架构为 Zynq PS 控制面 + PL 实时音频数据面。

本目录提供可综合的 24 位定点音频核、I2S RX/TX、NCO 合成器、DC blocker、饱和增益/混音、双声道 BRAM 延时、AXI4-Lite 参数寄存器、帧边界 CDC、自检 testbench，以及生成 XC7Z020 工程的 Tcl。外接 CODEC 未确定前只提交 XDC 模板，不臆造 BX71 音频引脚。

## 构建与验证

在仓库根目录执行：

```powershell
& 'E:\Vivado\2018.3\bin\vivado.bat' -mode batch -nojournal -nolog `
  -source '.\fpga\vivado\build_and_test.tcl'
```

脚本以 `xc7z020clg400-2` 创建工程，运行自检 XSim、综合、BRAM 推断门禁，并输出综合利用率、时序报告和 DCP 到被 Git 忽略的 `build/fpga-vivado-2018.3`。综合通过不等于可以生成板级 bitstream：外接 CODEC 管脚和 I/O 电压确定之前，故意不运行实现或 bitstream。

## 数据面

参考设计让外接 CODEC 做时钟主机，提供 3.072 MHz BCLK 和 48 kHz LRCLK；PL 音频数据面全在 BCLK 域运行，因此不需要让异步音频样本直接跨到 50 MHz 域。格式固定为立体声、24 个有效位、每声道 32 BCLK：

```text
I2S RX -> DC blocker -> Q3.23 gain -> NCO additive mix
       -> Q1.23 saturation -> stereo BRAM delay -> I2S TX
```

默认 16384 点/通道延时在 48 kHz 下约 341 ms，使用 24 个 RAMB36。NCO 每帧更新一次，相位字为：

```text
phase_inc = round(frequency_hz * 2^32 / sample_rate_hz)
```

## AXI4-Lite 寄存器

软件必须先写参数，再向 `CONTROL` 写 1 发起提交，并轮询 `CONTROL.pending` 变 0。提交未被音频域 ACK 时，新的写请求返回 `SLVERR`，保证 176 位参数总线在 CDC 握手期间稳定。

复位后的音频域参数总线为全零（安全静音）；即使寄存器块已有直通默认值，软件也必须至少执行一次 `CONTROL` 提交，音频才会解除静音。

| 偏移 | 名称 | 格式/说明 |
|---:|---|---|
| `0x00` | CONTROL | bit0 update toggle，bit1 ack，bit2 pending |
| `0x04` | GAIN | signed Q3.23，约 -4..+4 |
| `0x08` | DC_R | signed Q1.23，默认约 0.995 |
| `0x0C` | PHASE_INC | 32 位 NCO 相位增量 |
| `0x10` | OSC_SELECT | 0 saw，1 triangle，2 square，3 noise |
| `0x14` | SYNTH_LEVEL | signed Q1.23 |
| `0x18` | DELAY_SAMPLES | 1..16383 |
| `0x1C` | DELAY_FEEDBACK | signed Q1.23；软件应限制到约 ±0.95 |
| `0x20` | DELAY_MIX | Q1.23，0=dry，`0x7fffff`=wet |
| `0x24` | ID | 只读 `0x41554431` (`AUD1`) |

## 上板仍需完成

- 选外接 3.3 V 兼容的 I2S CODEC/ADC+DAC，并让其提供准确的 12.288 MHz MCLK、3.072 MHz BCLK、48 kHz LRCLK，或为另一时钟方案重做时钟/CDC。
- 实测所用 GPIO Bank 电压，再给 I2S 四根线补 XDC 管脚和 IOSTANDARD；按 CODEC 手册补 `set_input_delay` / `set_output_delay`。
- 在 Vivado block design 中把 PS GP0 接到 `aud_axi_lite_regs`，或把本 RTL封装成 AXI IP；软件可用裸机 MMIO、Linux UIO 或内核驱动控制。
- CODEC 的 I2C/SPI 初始化、静音/复位、模拟电源和低噪声时钟必须单独完成。
- Eurorack 双极性音频/CV 不能直连 FPGA：音频先经保护、衰减/增益和 CODEC，CV/Gate 使用独立 ADC/比较器及电平保护。
