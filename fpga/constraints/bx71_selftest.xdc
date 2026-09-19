# BX71 XC7Z020 board profile: every pin below is confirmed independently by
# the vendor manual, BX71 pin spreadsheet, and vendor ch01 LED example.

set_property PACKAGE_PIN U18 [get_ports pl_clk_50m]
set_property IOSTANDARD LVCMOS33 [get_ports pl_clk_50m]
create_clock -name pl_clk_50m -period 20.000 [get_ports pl_clk_50m]

# The standalone profile asserts sample_valid only once per 1041/1042 clock
# cycles.  Registers inside the DSP core hold between sample enables, so a
# conservative 1000-cycle exception describes the implemented clock-enable
# behavior while still leaving margin before the next 48 kHz sample.
set selftest_core_registers \
    [get_cells -hier -filter {NAME =~ core/* && IS_SEQUENTIAL}]
set_multicycle_path 1000 -setup \
    -from $selftest_core_registers -to $selftest_core_registers
set_multicycle_path 999 -hold \
    -from $selftest_core_registers -to $selftest_core_registers

set_property PACKAGE_PIN T19 [get_ports reset_n]
set_property IOSTANDARD LVCMOS33 [get_ports reset_n]
set_property PULLUP true [get_ports reset_n]
set_false_path -from [get_ports reset_n]

set_property PACKAGE_PIN T10 [get_ports fpga_led0]
set_property IOSTANDARD LVCMOS33 [get_ports fpga_led0]
set_property DRIVE 8 [get_ports fpga_led0]
set_property SLEW SLOW [get_ports fpga_led0]

set_property CONFIG_VOLTAGE 3.3 [current_design]
set_property CFGBVS VCCO [current_design]
