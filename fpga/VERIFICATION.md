# BX71 / Vivado 2018.3 verification

Verified on 2026-08-09 with Vivado v2018.3 build 2405991 and part `xc7z020clg400-2`:

- SystemVerilog self-checking simulation: `FPGA_RTL_TESTS_OK`;
- covered Q1.23/Q3.23 saturation, DC state, NCO injection, delay history, I2S receive alignment, independent AXI write channels, register reads, commit ACK and pending-write rejection;
- synthesis: 0 errors, 0 critical warnings;
- explicit memory gate: `FPGA_BRAM_INFERENCE_OK=1 RAMB36=24`, with zero RAM64M cells;
- utilization: 1225/53200 LUTs (2.30%), 794/106400 registers (0.75%), 24/140 block-RAM tiles (17.14%), and 22/220 DSP48 (10%);
- synthesis timing constraints include 50 MHz `pl_clk_50m` and 3.072 MHz `i2s_bclk`, separated as asynchronous clock groups; synthesis WNS was +17.281 ns and +161.379 ns respectively.

Not yet verified: placement/routing, external I2S pin assignment, codec input/output delays, bitstream generation, programming BX71, Zynq PS block design/software, electrical I/O voltage, real CODEC traffic, analog audio, latency, THD+N or long-run stability. Those require the selected external audio board and its actual connector mapping.
