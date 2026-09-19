# Simulation/synthesis/implementation driver for Vivado 2018.3.
# Usage: vivado -mode batch -source flow.tcl -tclargs selftest all

set script_dir [file dirname [file normalize [info script]]]
set fpga_dir [file normalize [file join $script_dir ..]]
set profile [expr {$argc >= 1 ? [lindex $argv 0] : "selftest"}]
set action [expr {$argc >= 2 ? [lindex $argv 1] : "all"}]

if {$action ni {create sim synth all}} {
    error "Unknown action '$action'; use create, sim, synth, or all"
}

set argv [list $profile]
set argc 1
source [file join $script_dir create_project.tcl]

set build_root [file join $fpga_dir build]
set report_dir [file join $build_root reports $profile]
file mkdir $report_dir

if {$action eq "create"} {
    puts "FPGA_FLOW_OK profile=$profile action=create"
    exit 0
}

if {$action in {sim all}} {
    launch_simulation -simset sim_1 -mode behavioral
    close_sim
    set sim_log [file join [get_property DIRECTORY [current_project]] \
        ${profile}.sim sim_1 behav xsim simulate.log]
    # The generated directory includes the project name rather than profile.
    set candidates [glob -nocomplain [file join [get_property DIRECTORY [current_project]] \
        *.sim sim_1 behav xsim simulate.log]]
    if {[llength $candidates] != 1} {
        error "Cannot locate XSim log under [get_property DIRECTORY [current_project]]"
    }
    set sim_log [lindex $candidates 0]
    set handle [open $sim_log r]
    set sim_text [read $handle]
    close $handle
    if {[string first "FPGA_RTL_TEST_FAIL" $sim_text] >= 0 ||
        [string first "FPGA_RTL_TESTS_OK" $sim_text] < 0} {
        error "Self-checking simulation failed; inspect $sim_log"
    }
    puts "FPGA_XSIM_OK profile=$profile"
    if {$action eq "sim"} {
        exit 0
    }
}

launch_runs synth_1 -jobs 4
wait_on_run synth_1
if {[get_property PROGRESS [get_runs synth_1]] ne "100%" ||
    ![string match "*Complete*" [get_property STATUS [get_runs synth_1]]]} {
    error "Synthesis failed: [get_property STATUS [get_runs synth_1]]"
}
open_run synth_1
report_utilization -file [file join $report_dir utilization_synth.rpt]
report_timing_summary -file [file join $report_dir timing_synth.rpt]
set ram36 [llength [get_cells -quiet -hier -filter {REF_NAME =~ RAMB36*}]]
set dsp48 [llength [get_cells -quiet -hier -filter {REF_NAME =~ DSP48*}]]
set ram64m [llength [get_cells -quiet -hier -filter {REF_NAME =~ RAM64M*}]]
puts "FPGA_SYNTH_OK profile=$profile RAMB36=$ram36 DSP48=$dsp48 RAM64M=$ram64m"

if {$ram36 < 24} {
    error "Audio delay BRAM regression: expected at least 24 RAMB36, got $ram36"
}
if {$ram64m != 0} {
    error "Audio delay distributed-memory regression: RAM64M=$ram64m"
}

if {$action eq "synth" || $profile eq "i2s_external"} {
    if {$profile eq "i2s_external" && $action eq "all"} {
        puts "FPGA_BITSTREAM_BLOCKED profile=$profile reason=external_i2s_board_profile_required"
    }
    exit 0
}

if {$profile eq "soc_i2s"} {
    # Project-mode impl_1 re-adds the BD and, in Vivado 2018.3, ignores the
    # generated PS7 XDC's used_in_implementation=false property.  Implement the
    # already stitched/open synthesis design so the obsolete DDR/MIO property
    # replay cannot enter the routed checkpoint.
    set ps7_xdc [get_files -quiet -all -filter \
        {FILE_TYPE == XDC && NAME =~ *processing_system7_0_0.xdc}]
    if {[llength $ps7_xdc] != 1 ||
        [get_property used_in_implementation [lindex $ps7_xdc 0]]} {
        error "PS7 full I/O XDC is not safely excluded from SoC implementation"
    }
    opt_design -directive Explore
    place_design -directive Explore
    phys_opt_design -directive Explore
    route_design -directive Explore
    puts "FPGA_SOC_DIRECT_IMPL_OK ps7_full_xdc_excluded=1"
} else {
    close_design
    launch_runs impl_1 -to_step write_bitstream -jobs 4
    wait_on_run impl_1
    if {[get_property PROGRESS [get_runs impl_1]] ne "100%" ||
        ![string match "*Complete*" [get_property STATUS [get_runs impl_1]]]} {
        error "Implementation failed: [get_property STATUS [get_runs impl_1]]"
    }
    open_run impl_1

    set impl_run_log [file join [get_property DIRECTORY [current_project]] \
        *.runs impl_1 runme.log]
    set impl_log_candidates [glob -nocomplain $impl_run_log]
    if {[llength $impl_log_candidates] != 1} {
        error "Cannot locate unique implementation run log"
    }
    set impl_log_handle [open [lindex $impl_log_candidates 0] r]
    set impl_log_text [read $impl_log_handle]
    close $impl_log_handle
    if {[string first "Netlist 29-160" $impl_log_text] >= 0 ||
        [string first "Common 17-55" $impl_log_text] >= 0} {
        error "Implementation contains invalid scoped-I/O constraints"
    }
}

report_drc -file [file join $report_dir drc_impl.rpt]
report_utilization -file [file join $report_dir utilization_impl.rpt]
report_timing_summary -check_timing_verbose \
    -file [file join $report_dir timing_impl.rpt]

set drc_error_count 0
set drc_critical_count 0
set drc_warning_count 0
set drc_advisory_count 0
set drc_violations [get_drc_violations -quiet]
foreach violation $drc_violations {
    set severity [get_property SEVERITY $violation]
    if {$severity eq "Error"} {
        incr drc_error_count
    } elseif {$severity eq "Critical Warning"} {
        incr drc_critical_count
    } elseif {$severity eq "Warning"} {
        incr drc_warning_count
    } elseif {$severity eq "Advisory"} {
        incr drc_advisory_count
    }
}
if {$drc_error_count != 0 || $drc_critical_count != 0} {
    error "Implementation DRC failed: errors=$drc_error_count critical_warnings=$drc_critical_count"
}

set setup_path [get_timing_paths -quiet -delay_type max -max_paths 1]
set hold_path [get_timing_paths -quiet -delay_type min -max_paths 1]
if {[llength $setup_path] == 0 || [llength $hold_path] == 0} {
    error "No setup/hold timing paths found"
}
set wns [get_property SLACK [lindex $setup_path 0]]
set whs [get_property SLACK [lindex $hold_path 0]]
if {$wns < 0.0 || $whs < 0.0} {
    error "Timing failed: WNS=$wns WHS=$whs"
}

set output_dir [file join $build_root output]
file mkdir $output_dir
set output_bit [file join $output_dir audio_dsp_bx71_${profile}.bit]
if {$profile eq "soc_i2s"} {
    write_bitstream -force $output_bit
} else {
    set bit_candidates [glob -nocomplain \
        [file join [get_property DIRECTORY [current_project]] *.runs impl_1 *.bit]]
    if {[llength $bit_candidates] != 1} {
        error "Expected exactly one bitstream, found [llength $bit_candidates]"
    }
    file copy -force [lindex $bit_candidates 0] $output_bit
}
if {[file size $output_bit] == 0} {
    error "Generated bitstream is empty: $output_bit"
}
if {$profile eq "soc_i2s"} {
    set output_hdf [file join $output_dir audio_dsp_bx71_${profile}.hdf]
    # Vivado 2018.3 uses write_hwdef for the SDK hardware handoff; the newer
    # -include_bit switch belongs to write_hw_platform and is not available.
    write_hwdef -force -file $output_hdf
    if {![file exists $output_hdf] || [file size $output_hdf] == 0} {
        error "Generated HDF is missing or empty: $output_hdf"
    }
    puts "FPGA_HDF_OK file=$output_hdf bytes=[file size $output_hdf]"
}
set impl_luts [llength [get_cells -quiet -hier -filter {REF_NAME =~ LUT*}]]
set impl_ffs [llength [get_cells -quiet -hier -filter {REF_NAME =~ FD*}]]
set impl_ram36 [llength [get_cells -quiet -hier -filter {REF_NAME =~ RAMB36*}]]
set impl_dsp48 [llength [get_cells -quiet -hier -filter {REF_NAME =~ DSP48*}]]
set impl_ram64m [llength [get_cells -quiet -hier -filter {REF_NAME =~ RAM64M*}]]
if {$impl_ram36 < 24 || $impl_ram64m != 0} {
    error "Implemented delay-memory regression: RAMB36=$impl_ram36 RAM64M=$impl_ram64m"
}
puts "FPGA_IMPLEMENTATION_OK profile=$profile WNS=$wns WHS=$whs DRC_TOTAL=[llength $drc_violations] DRC_ERRORS=0 DRC_CRITICAL_WARNINGS=0 DRC_WARNINGS=$drc_warning_count DRC_ADVISORIES=$drc_advisory_count LUT_CELLS=$impl_luts FF_CELLS=$impl_ffs RAMB36=$impl_ram36 DSP48=$impl_dsp48"
puts "FPGA_BITSTREAM_OK file=$output_bit bytes=[file size $output_bit]"
exit 0
