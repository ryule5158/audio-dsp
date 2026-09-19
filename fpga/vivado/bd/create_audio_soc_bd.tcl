# Optional PS-control profile.  The PS uses OCM/JTAG and intentionally does not
# configure board DDR, so this template cannot silently apply a DDR preset for
# the wrong BX71 hardware revision.  Import the matching vendor PS preset when
# moving to Linux/DDR.

create_bd_design audio_soc

set i2s_bclk [create_bd_port -dir I -type clk i2s_bclk]
set_property CONFIG.FREQ_HZ 3072000 $i2s_bclk
set wm8960_mclk [create_bd_port -dir I -type clk wm8960_mclk]
set_property CONFIG.FREQ_HZ 24000000 $wm8960_mclk
set i2s_lrclk [create_bd_port -dir I i2s_lrclk]
set i2s_adc_data [create_bd_port -dir I i2s_adc_data]
set i2s_dac_data [create_bd_port -dir O i2s_dac_data]

set ps [create_bd_cell -type ip \
    -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0]
set_property -dict [list \
    CONFIG.PCW_EN_DDR {0} \
    CONFIG.PCW_USE_M_AXI_GP0 {1} \
    CONFIG.PCW_EN_CLK0_PORT {1} \
    CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ {50.000000} \
    CONFIG.PCW_CRYSTAL_PERIPHERAL_FREQMHZ {33.333333} \
    CONFIG.PCW_I2C0_PERIPHERAL_ENABLE {1} \
    CONFIG.PCW_I2C0_I2C0_IO {EMIO} \
    CONFIG.PCW_EN_EMIO_I2C0 {1} \
    CONFIG.PCW_UART1_PERIPHERAL_ENABLE {1} \
    CONFIG.PCW_UART1_UART1_IO {MIO 48 .. 49}] $ps

# Let IP Integrator build the external interfaces so all dedicated-PS and IIC
# pin metadata is retained in the generated wrapper and scoped constraints.
make_bd_intf_pins_external [get_bd_intf_pins $ps/FIXED_IO]
set fixed_io [get_bd_intf_ports FIXED_IO_0]
set_property name FIXED_IO $fixed_io
make_bd_intf_pins_external [get_bd_intf_pins $ps/IIC_0]
set wm8960_iic [get_bd_intf_ports IIC_0_0]
set_property name wm8960_iic $wm8960_iic

set interconnect [create_bd_cell -type ip \
    -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_0]
set_property CONFIG.NUM_MI 1 $interconnect

set reset [create_bd_cell -type ip \
    -vlnv xilinx.com:ip:proc_sys_reset:5.0 rst_ps7_0_50M]

set audio [create_bd_cell -type module -reference aud_soc_audio_block audio_dsp_0]

connect_bd_intf_net [get_bd_intf_pins $ps/M_AXI_GP0] \
    [get_bd_intf_pins $interconnect/S00_AXI]
connect_bd_intf_net [get_bd_intf_pins $interconnect/M00_AXI] \
    [get_bd_intf_pins $audio/S_AXI]

connect_bd_net [get_bd_pins $ps/FCLK_CLK0] \
    [get_bd_pins $ps/M_AXI_GP0_ACLK] \
    [get_bd_pins $interconnect/ACLK] \
    [get_bd_pins $interconnect/S00_ACLK] \
    [get_bd_pins $interconnect/M00_ACLK] \
    [get_bd_pins $reset/slowest_sync_clk] \
    [get_bd_pins $audio/S_AXI_ACLK]
connect_bd_net [get_bd_pins $ps/FCLK_RESET0_N] \
    [get_bd_pins $reset/ext_reset_in]
connect_bd_net [get_bd_pins $reset/interconnect_aresetn] \
    [get_bd_pins $interconnect/ARESETN] \
    [get_bd_pins $interconnect/S00_ARESETN] \
    [get_bd_pins $interconnect/M00_ARESETN]
connect_bd_net [get_bd_pins $reset/peripheral_aresetn] \
    [get_bd_pins $audio/S_AXI_ARESETN]

connect_bd_net $i2s_bclk [get_bd_pins $audio/i2s_bclk]
connect_bd_net $wm8960_mclk [get_bd_pins $audio/wm8960_mclk]
connect_bd_net $i2s_lrclk [get_bd_pins $audio/i2s_lrclk]
connect_bd_net $i2s_adc_data [get_bd_pins $audio/i2s_adc_data]
connect_bd_net $i2s_dac_data [get_bd_pins $audio/i2s_dac_data]

assign_bd_address

validate_bd_design
save_bd_design
puts "FPGA_SOC_BD_CREATED base=0x43C00000 ps_memory=OCM"
