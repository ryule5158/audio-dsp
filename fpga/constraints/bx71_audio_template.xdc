# Confirmed by the BX71 XC7Z020 vendor example: PL oscillator is 50 MHz on U18.
set_property PACKAGE_PIN U18 [get_ports pl_clk_50m]
set_property IOSTANDARD LVCMOS33 [get_ports pl_clk_50m]
create_clock -name pl_clk_50m -period 20.000 [get_ports pl_clk_50m]

# Default codec-master format: 48 kHz, stereo, 32 BCLKs/channel.
create_clock -name i2s_bclk -period 325.521 [get_ports i2s_bclk]
set_clock_groups -asynchronous \
    -group [get_clocks pl_clk_50m] -group [get_clocks i2s_bclk]

# Confirmed board reset/key pin from the same vendor example.
set_property PACKAGE_PIN T19 [get_ports reset_n]
set_property IOSTANDARD LVCMOS33 [get_ports reset_n]

# Do not uncomment or invent the I2S pins. Bind these ports only after choosing
# the external codec connector and measuring the selected GPIO bank voltage:
# set_property PACKAGE_PIN <PIN> [get_ports i2s_bclk]
# set_property PACKAGE_PIN <PIN> [get_ports i2s_lrclk]
# set_property PACKAGE_PIN <PIN> [get_ports i2s_adc_data]
# set_property PACKAGE_PIN <PIN> [get_ports i2s_dac_data]

# ADC-data input delay and DAC-data output delay depend on the selected codec
# timing table and PCB trace. Add set_input_delay/set_output_delay only after
# those values are known.
