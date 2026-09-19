//Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2018.3 (win64) Build 2405991 Thu Dec  6 23:38:27 MST 2018
//Date        : Fri Sep 11 11:35:17 2026
//Host        : LAPTOP-9SJO8I4N running 64-bit major release  (build 9200)
//Command     : generate_target audio_soc_wrapper.bd
//Design      : audio_soc_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module audio_soc_wrapper
   (FIXED_IO_mio,
    FIXED_IO_ps_clk,
    FIXED_IO_ps_porb,
    FIXED_IO_ps_srstb,
    i2s_adc_data,
    i2s_bclk,
    i2s_dac_data,
    i2s_lrclk,
    wm8960_iic_scl_io,
    wm8960_iic_sda_io,
    wm8960_mclk);
  inout [53:0]FIXED_IO_mio;
  inout FIXED_IO_ps_clk;
  inout FIXED_IO_ps_porb;
  inout FIXED_IO_ps_srstb;
  input i2s_adc_data;
  input i2s_bclk;
  output i2s_dac_data;
  input i2s_lrclk;
  inout wm8960_iic_scl_io;
  inout wm8960_iic_sda_io;
  input wm8960_mclk;

  wire [53:0]FIXED_IO_mio;
  wire FIXED_IO_ps_clk;
  wire FIXED_IO_ps_porb;
  wire FIXED_IO_ps_srstb;
  wire i2s_adc_data;
  wire i2s_bclk;
  wire i2s_dac_data;
  wire i2s_lrclk;
  wire wm8960_iic_scl_i;
  wire wm8960_iic_scl_io;
  wire wm8960_iic_scl_o;
  wire wm8960_iic_scl_t;
  wire wm8960_iic_sda_i;
  wire wm8960_iic_sda_io;
  wire wm8960_iic_sda_o;
  wire wm8960_iic_sda_t;
  wire wm8960_mclk;

  audio_soc audio_soc_i
       (.FIXED_IO_mio(FIXED_IO_mio),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),
        .i2s_adc_data(i2s_adc_data),
        .i2s_bclk(i2s_bclk),
        .i2s_dac_data(i2s_dac_data),
        .i2s_lrclk(i2s_lrclk),
        .wm8960_iic_scl_i(wm8960_iic_scl_i),
        .wm8960_iic_scl_o(wm8960_iic_scl_o),
        .wm8960_iic_scl_t(wm8960_iic_scl_t),
        .wm8960_iic_sda_i(wm8960_iic_sda_i),
        .wm8960_iic_sda_o(wm8960_iic_sda_o),
        .wm8960_iic_sda_t(wm8960_iic_sda_t),
        .wm8960_mclk(wm8960_mclk));
  IOBUF wm8960_iic_scl_iobuf
       (.I(wm8960_iic_scl_o),
        .IO(wm8960_iic_scl_io),
        .O(wm8960_iic_scl_i),
        .T(wm8960_iic_scl_t));
  IOBUF wm8960_iic_sda_iobuf
       (.I(wm8960_iic_sda_o),
        .IO(wm8960_iic_sda_io),
        .O(wm8960_iic_sda_i),
        .T(wm8960_iic_sda_t));
endmodule
