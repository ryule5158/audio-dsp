# CubeMX 配置清单

器件选择 `STM32H743IIT6`（LQFP176），工程工具链选择 `MDK-ARM`，生成代码保留 USER CODE。以下为 48 kHz、24-bit stereo、H743 主时钟方案的基准；最终 GPIO 必须与自制 CODEC 原理图一致。

## SAI1

- Block A：Master Transmitter，异步，I2S/Philips，24-bit data，32-bit slot，2 slots，frame 64 bit，FS active 32 bit，MCLK output enabled。
- Block B：Slave Receiver，Synchronous with Block A，24-bit data，32-bit slot，2 slots。
- 可用 LQFP176 参考映射：PE2/SAI1_MCLK_A、PE4/SAI1_FS_A、PE5/SAI1_SCK_A、PE6/SAI1_SD_A、PE3/SAI1_SD_B。CubeMX 出现冲突时以器件 pinout 重新选择同一 AF。
- SAI kernel clock 必须产生精确音频倍频；验证 MCLK 实测 12.288 MHz，不能只看 CubeMX 显示的近似采样率。

## DMA/NVIC

- SAI1_A TX 与 SAI1_B RX 各使用一个 DMA stream，Peripheral/Memory width 都为 word，memory increment，circular，priority very high。
- 两个 DMA IRQ 使用相同高优先级；不要在比音频 DMA 更高的中断里做长耗时工作。
- HAL 的 `Size` 参数按 SAI 数据 word 计；本库一整个双缓冲为 `4 * half_frames` 个 32-bit word。

## I2C/CODEC

- I2C1 400 kHz 作为 CODEC 控制口（例如 PB8/PB9）。寄存器脚本必须与所选 CODEC 和晶振/主从模式一致。
- 启动顺序：电源稳定 -> I2C 配置 CODEC 且保持 mute -> 启动 SAI RX DMA -> 启动 SAI TX DMA/时钟 -> 清零若干帧 -> 解除 mute。

## Cache 与内存

- DMA RX/TX 缓冲 32-byte 对齐并放到 DMA 可见 SRAM；示例使用 `.AudioDmaBuffer` section。
- 推荐 MPU 把该 section 设为 non-cacheable；若保留 cacheable，必须使用 `aud_mcu_sai_dma.c` 中的 invalidate/clean 路径。
- 大延时、Clouds/Rings/Elements 等状态放 AXI SRAM/SDRAM，不放 128 KiB DTCM；确认链接 map 中的真实地址。

## Keil

- ArmClang 6，C11，Cortex-M7，FPv5-D16 hard-float，优化 `-O2` 起步。
- 运行 `mcu/keil/build_armclang.ps1` 可生成并审计独立静态库；CubeMX 工程中加入该 `.lib`、根目录和 `mcu/include` include path。
- CMSIS-DSP 通过 Manage Run-Time Environment/Pack 启用，定义 `ARM_MATH_CM7`；不要再复制另一份 CMSIS 到仓库。
