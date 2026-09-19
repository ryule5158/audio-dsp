# Opt-in profile: Waveshare WM8960 Audio Board on BX71 P2/GPIO0.
# GPIO0 is fixed 3.3 V. Pins are from BX71 manual Figure 39/Table 15 and the
# vendor pin spreadsheet.
set_property PACKAGE_PIN U18 [get_ports pl_clk_50m]
set_property IOSTANDARD LVCMOS33 [get_ports pl_clk_50m]
create_clock -name pl_clk_50m -period 20.000 [get_ports pl_clk_50m]
set_property PACKAGE_PIN T19 [get_ports reset_n]
set_property IOSTANDARD LVCMOS33 [get_ports reset_n]
set_property PULLUP true [get_ports reset_n]
set_false_path -from [get_ports reset_n]
set_property PACKAGE_PIN T10 [get_ports fpga_led0]
set_property IOSTANDARD LVCMOS33 [get_ports fpga_led0]
set_property DRIVE 8 [get_ports fpga_led0]
set_property SLEW SLOW [get_ports fpga_led0]

# Waveshare P1 jumper 1-2 selects its onboard 24 MHz oscillator.
# P2-25 / GPIO0_22 / J18 is an SRCC clock-capable input.
set_property PACKAGE_PIN J18 [get_ports wm8960_mclk]
set_property IOSTANDARD LVCMOS33 [get_ports wm8960_mclk]
create_clock -name wm8960_mclk -period 41.667 [get_ports wm8960_mclk]

# Codec-master I2S. BCLK is on P2-33 / GPIO0_28 / K17 (MRCC).
set_property PACKAGE_PIN K17 [get_ports i2s_bclk]
set_property PACKAGE_PIN H15 [get_ports i2s_lrclk]
set_property PACKAGE_PIN F16 [get_ports i2s_adc_data]
set_property PACKAGE_PIN E17 [get_ports i2s_dac_data]
set_property IOSTANDARD LVCMOS33 \
    [get_ports {i2s_bclk i2s_lrclk i2s_adc_data i2s_dac_data}]
set_property DRIVE 8 [get_ports i2s_dac_data]
set_property SLEW SLOW [get_ports i2s_dac_data]
create_clock -name i2s_bclk -period 325.521 [get_ports i2s_bclk]

# WM8960 rev4.4 master timing: LRCLK/ADCDAT max 10 ns from BCLK falling;
# DACDAT setup and hold are both 10 ns at BCLK rising.
set_input_delay -clock i2s_bclk -clock_fall -min 0.000 \
    [get_ports {i2s_lrclk i2s_adc_data}]
set_input_delay -clock i2s_bclk -clock_fall -max 10.000 \
    [get_ports {i2s_lrclk i2s_adc_data}]
set_output_delay -clock i2s_bclk -min -10.000 [get_ports i2s_dac_data]
set_output_delay -clock i2s_bclk -max 10.000 [get_ports i2s_dac_data]
set_clock_groups -asynchronous \
    -group [get_clocks pl_clk_50m] \
    -group [get_clocks wm8960_mclk] \
    -group [get_clocks i2s_bclk]
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]
