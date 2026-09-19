# Rebuildable Vivado 2018.3 project entry point.
# Usage: vivado -mode batch -source create_project.tcl -tclargs selftest ?project_root?

set script_dir [file dirname [file normalize [info script]]]
set fpga_dir [file normalize [file join $script_dir ..]]
set profile [expr {$argc >= 1 ? [lindex $argv 0] : "selftest"}]
set project_root [expr {$argc >= 2 ? [file normalize [lindex $argv 1]] :
    [file join $fpga_dir build]}]

if {$profile ni {selftest wm8960_waveshare i2s_external soc_i2s}} {
    error "Unknown profile '$profile'; use selftest, wm8960_waveshare, i2s_external, or soc_i2s"
}

set project_name audio_dsp_bx71_${profile}
set project_dir [file join $project_root $project_name]
file mkdir [file dirname $project_dir]
create_project -force $project_name $project_dir -part xc7z020clg400-2
set_property target_language Verilog [current_project]
set_property simulator_language Mixed [current_project]
set_property default_lib xil_defaultlib [current_project]
set_property ip_cache_permissions {read write} [current_project]

set rtl_files [lsort [glob [file join $fpga_dir rtl *.v]]]
add_files -norecurse $rtl_files
# aud_fixed_math.vh contains module-local functions and is intentionally only
# reached through `include directives.  Keep it in the project so the 2018.3
# module-reference resolver can elaborate aud_soc_audio_block, but explicitly
# exclude it as a standalone synthesis/simulation compilation unit.
set header_files [lsort [glob -nocomplain [file join $fpga_dir rtl *.vh]]]
if {[llength $header_files] > 0} {
    add_files -norecurse $header_files
    set_property file_type {Verilog Header} [get_files $header_files]
    set_property used_in_synthesis false [get_files $header_files]
    set_property used_in_simulation false [get_files $header_files]
}
set_property include_dirs [list [file join $fpga_dir rtl]] [current_fileset]

add_files -fileset sim_1 -norecurse [file join $fpga_dir tb tb_audio_dsp.sv]
set_property top tb_audio_dsp [get_filesets sim_1]
set_property xsim.simulate.runtime 100us [get_filesets sim_1]
update_compile_order -fileset sources_1

switch -- $profile {
    selftest {
        set_property top aud_bx71_selftest_top [get_filesets sources_1]
        add_files -fileset constrs_1 -norecurse \
            [file join $fpga_dir constraints bx71_selftest.xdc]
    }
    i2s_external {
        set_property top aud_i2s_demo_top [get_filesets sources_1]
        add_files -fileset constrs_1 -norecurse \
            [file join $fpga_dir constraints bx71_i2s_external_unassigned.xdc]
    }
    wm8960_waveshare {
        set_property top aud_i2s_demo_top [get_filesets sources_1]
        add_files -fileset constrs_1 -norecurse \
            [file join $fpga_dir constraints bx71_wm8960_waveshare_gpio0.xdc]
    }
    soc_i2s {
        set_property top aud_soc_audio_block [get_filesets sources_1]
        update_compile_order -fileset sources_1
        source [file join $script_dir bd create_audio_soc_bd.tcl]
        set bd_file [get_files -quiet */audio_soc.bd]
        if {[llength $bd_file] != 1} {
            error "audio_soc.bd was not generated"
        }
        generate_target all $bd_file
        # Vivado 2018.3 still emits the PS7 MIO/DDR package-pin XDC when DDR is
        # intentionally disabled.  In a hierarchical BD implementation that
        # file is scoped to PS7 cell pins, so its top-port I/O properties cause
        # Netlist 29-160 critical warnings.  The PS pins are dedicated silicon
        # pins and the FCLK constraint is retained in the PS7 OOC checkpoint;
        # exclude only this obsolete implementation-time I/O-property replay.
        set ps7_xdc [get_files -quiet -all -filter \
            {FILE_TYPE == XDC && NAME =~ *processing_system7_0_0.xdc}]
        if {[llength $ps7_xdc] != 1} {
            error "Expected exactly one generated PS7 XDC, found [llength $ps7_xdc]"
        }
        set_property used_in_implementation false $ps7_xdc
        puts "FPGA_PS7_XDC_IMPL_EXCLUDED file=[lindex $ps7_xdc 0]"
        set wrapper [make_wrapper -files $bd_file -top]
        add_files -norecurse $wrapper
        set_property top audio_soc_wrapper [get_filesets sources_1]
        add_files -fileset constrs_1 -norecurse \
            [file join $fpga_dir constraints bx71_soc_wm8960_waveshare_gpio0.xdc]
    }
}

update_compile_order -fileset sources_1
update_compile_order -fileset sim_1
set_property strategy Flow_PerfOptimized_high [get_runs synth_1]
set_property strategy Performance_Explore [get_runs impl_1]
puts "FPGA_PROJECT_CREATED profile=$profile xpr=[get_property DIRECTORY [current_project]]/$project_name.xpr"
