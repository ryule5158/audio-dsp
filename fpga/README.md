# FPGA/SoC library - BX71 XC7Z020

目标工具链：Vivado 2018.3。架构为 Zynq PS 控制面 + PL 实时音频数据面。

本目录将提供可参数化 I2S RX/TX、定点流接口、饱和/增益/混音、NCO、滤波与 BRAM 延时模块，自检 testbench，以及生成 XC7Z020 工程的 Tcl。外接 CODEC 未确定前只提交 XDC 模板，不臆造 BX71 音频引脚。

当前阶段：BX71 时钟和扩展 Bank 事实已从用户手册核对，RTL 与 Vivado 验证将在后续阶段加入。
