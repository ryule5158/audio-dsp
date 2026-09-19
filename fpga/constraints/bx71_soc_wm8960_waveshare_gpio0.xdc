# Zynq PS + AXI + PS-I2C0/EMIO profile for the Waveshare WM8960 Audio Board.
# GPIO0 is the BX71 fixed 3.3 V bank. The P2 mapping is taken from BX71 manual
# Figure 39/Table 15 and cross-checked against the vendor pin spreadsheet.

# Waveshare onboard 24 MHz MCLK -> BX71 P2-25 / GPIO0_22 / SRCC.
set_property PACKAGE_PIN J18 [get_ports wm8960_mclk]
set_property IOSTANDARD LVCMOS33 [get_ports wm8960_mclk]
create_clock -name wm8960_mclk -period 41.667 [get_ports wm8960_mclk]

# WM8960 codec-master I2S. BCLK uses P2-33 / GPIO0_28 / MRCC.
set_property PACKAGE_PIN K17 [get_ports i2s_bclk]
set_property PACKAGE_PIN H15 [get_ports i2s_lrclk]
set_property PACKAGE_PIN F16 [get_ports i2s_adc_data]
set_property PACKAGE_PIN E17 [get_ports i2s_dac_data]
set_property IOSTANDARD LVCMOS33 \
    [get_ports {i2s_bclk i2s_lrclk i2s_adc_data i2s_dac_data}]
set_property DRIVE 8 [get_ports i2s_dac_data]
set_property SLEW SLOW [get_ports i2s_dac_data]
create_clock -name i2s_bclk -period 325.521 [get_ports i2s_bclk]
set_input_delay -clock i2s_bclk -clock_fall -min 0.000 \
    [get_ports {i2s_lrclk i2s_adc_data}]
set_input_delay -clock i2s_bclk -clock_fall -max 10.000 \
    [get_ports {i2s_lrclk i2s_adc_data}]
set_output_delay -clock i2s_bclk -min -10.000 [get_ports i2s_dac_data]
set_output_delay -clock i2s_bclk -max 10.000 [get_ports i2s_dac_data]

# PS7 I2C0 through EMIO and generated IOBUFs: P2-13/P2-14.
set_property PACKAGE_PIN B19 [get_ports wm8960_iic_sda_io]
set_property PACKAGE_PIN A20 [get_ports wm8960_iic_scl_io]
set_property IOSTANDARD LVCMOS33 \
    [get_ports {wm8960_iic_sda_io wm8960_iic_scl_io}]
set_property PULLUP true \
    [get_ports {wm8960_iic_sda_io wm8960_iic_scl_io}]

# During top synthesis Vivado reads this user XDC before it elaborates the BD
# and its PS7 OOC clock.  Implementation replays it after the OOC checkpoint is
# linked.  Quiet lookup suppresses that synthesis-only false 12-4739; after
# checkpoint linking, implementation finds clk_fpga_0 and applies the groups.
set_clock_groups -quiet -asynchronous \
    -group [get_clocks -quiet clk_fpga_0] \
    -group [get_clocks wm8960_mclk] \
    -group [get_clocks i2s_bclk]
set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]
