set script_dir [file dirname [file normalize [info script]]]
set fpga_dir [file normalize [file join $script_dir ..]]
set repo_root [file normalize [file join $fpga_dir ..]]
set build_dir [file join $repo_root build fpga-vivado-2018.3]

file mkdir $build_dir
create_project -force audio_dsp_bx71 $build_dir -part xc7z020clg400-2
set_property target_language Verilog [current_project]
set_property simulator_language Mixed [current_project]

set rtl_files [glob [file join $fpga_dir rtl *.v]]
add_files -norecurse $rtl_files
set_property include_dirs [list [file join $fpga_dir rtl]] [current_fileset]
add_files -fileset constrs_1 -norecurse \
    [file join $fpga_dir constraints bx71_audio_template.xdc]
add_files -fileset sim_1 -norecurse [file join $fpga_dir tb tb_audio_dsp.sv]
set_property top tb_audio_dsp [get_filesets sim_1]
set_property xsim.simulate.runtime 10us [get_filesets sim_1]
set_property top aud_bx71_top [get_filesets sources_1]

launch_simulation
close_sim

set sim_log [file join $build_dir audio_dsp_bx71.sim sim_1 behav xsim simulate.log]
set log_handle [open $sim_log r]
set log_text [read $log_handle]
close $log_handle
if {[string first "FPGA_RTL_TEST_FAIL" $log_text] >= 0 ||
    [string first "FPGA_RTL_TESTS_OK" $log_text] < 0} {
    error "FPGA self-checking simulation failed; inspect $sim_log"
}
puts "FPGA_XSIM_TESTS_OK=1"

synth_design -top aud_bx71_top -part xc7z020clg400-2
set block_ram_count [llength [get_cells -hier -filter {REF_NAME =~ RAMB36*}]]
set distributed_ram_count [llength [get_cells -quiet -hier -filter {REF_NAME =~ RAM64M*}]]
if {$block_ram_count < 24 || $distributed_ram_count != 0} {
    error "Delay memory resource regression: RAMB36=$block_ram_count RAM64M=$distributed_ram_count"
}
puts "FPGA_BRAM_INFERENCE_OK=1 RAMB36=$block_ram_count"
report_utilization -file [file join $build_dir utilization_synth.rpt]
report_timing_summary -file [file join $build_dir timing_synth.rpt]
write_checkpoint -force [file join $build_dir aud_bx71_top_synth.dcp]

puts "FPGA_VIVADO_2018_3_SYNTH_OK=1"
exit
