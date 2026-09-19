set script_dir [file dirname [file normalize [info script]]]
set fpga_dir [file normalize [file join $script_dir ..]]
open_checkpoint [file join $fpga_dir build audio_dsp_bx71_selftest \
    audio_dsp_bx71_selftest.runs impl_1 aud_bx71_selftest_top_routed.dcp]
set gpio0_pins {
    L14 L15 K14 J14 H15 G15 F16 F17 E17 D18 B19 A20 C20 B20
    D19 D20 E18 E19 F19 F20 G19 G20 J18 H18 J20 H20 K19 J19
    K17 K18 L16 L17 L19 L20 M19 M20
}
foreach package_pin $gpio0_pins {
    set pin [get_package_pins -quiet $package_pin]
    puts "BX71_PIN $package_pin pin_func=[get_property PIN_FUNC $pin] bank=[get_property BANK $pin]"
}
exit 0
