set script_dir [file dirname [file normalize [info script]]]
set fpga_dir [file normalize [file join $script_dir ..]]
open_project [file join $fpga_dir build audio_dsp_bx71_soc_i2s \
    audio_dsp_bx71_soc_i2s.xpr]
open_run impl_1
set output_hdf [file join $fpga_dir build output audio_dsp_bx71_soc_i2s.hdf]
write_hwdef -force -file $output_hdf
puts "FPGA_HDF_OK file=$output_hdf"
exit 0
