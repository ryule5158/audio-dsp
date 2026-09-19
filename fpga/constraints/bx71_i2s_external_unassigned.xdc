# Known BX71 on-board resources.  These are sufficient for synthesis, but this
# profile MUST NOT proceed to implementation/bitstream until the I2S section is
# completed for the actual external codec board.
set_property PACKAGE_PIN U18 [get_ports pl_clk_50m]
set_property IOSTANDARD LVCMOS33 [get_ports pl_clk_50m]
create_clock -name pl_clk_50m -period 20.000 [get_ports pl_clk_50m]

set_property PACKAGE_PIN T19 [get_ports reset_n]
set_property IOSTANDARD LVCMOS33 [get_ports reset_n]
set_property PULLUP true [get_ports reset_n]
set_false_path -from [get_ports reset_n]

set_property PACKAGE_PIN T10 [get_ports fpga_led0]
set_property IOSTANDARD LVCMOS33 [get_ports fpga_led0]

# Codec-master example: 48 kHz stereo with 32-bit slots -> 3.072 MHz BCLK.
create_clock -name i2s_bclk -period 325.521 [get_ports i2s_bclk]
create_clock -name wm8960_mclk -period 41.667 [get_ports wm8960_mclk]
set_clock_groups -asynchronous -group [get_clocks pl_clk_50m] \
    -group [get_clocks i2s_bclk] -group [get_clocks wm8960_mclk]

# BOARD_PROFILE_REQUIRED_BEGIN
# set_property PACKAGE_PIN <MEASURED_CONNECTOR_PIN> [get_ports i2s_bclk]
# set_property PACKAGE_PIN <MEASURED_CONNECTOR_PIN> [get_ports wm8960_mclk]
# set_property PACKAGE_PIN <MEASURED_CONNECTOR_PIN> [get_ports i2s_lrclk]
# set_property PACKAGE_PIN <MEASURED_CONNECTOR_PIN> [get_ports i2s_adc_data]
# set_property PACKAGE_PIN <MEASURED_CONNECTOR_PIN> [get_ports i2s_dac_data]
# set_property IOSTANDARD <BANK_VOLTAGE_STANDARD> \
#     [get_ports {i2s_bclk i2s_lrclk i2s_adc_data i2s_dac_data}]
# Add set_input_delay/set_output_delay from the selected codec timing table.
# BOARD_PROFILE_REQUIRED_END
