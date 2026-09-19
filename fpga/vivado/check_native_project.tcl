# Open a checked-in project exactly as a user would, then reject missing or
# out-of-repository sources. Usage:
#   vivado -mode batch -source check_native_project.tcl -tclargs selftest

set script_dir [file dirname [file normalize [info script]]]
set fpga_dir [file normalize [file join $script_dir ..]]
set profile [expr {$argc >= 1 ? [lindex $argv 0] : "selftest"}]

if {$profile ni {selftest soc_i2s}} {
    error "Native project profile must be selftest or soc_i2s"
}

set project_name audio_dsp_bx71_${profile}
set project_path [file join $fpga_dir projects $project_name ${project_name}.xpr]
if {![file exists $project_path]} {
    error "Missing checked-in project: $project_path"
}

set project_handle [open $project_path r]
set project_text [read $project_handle]
close $project_handle
if {![regexp {<Project[^>]* Path="([^"]+)"} $project_text unused \
        project_metadata_path]} {
    error "Cannot read root Project Path metadata from $project_path"
}
set expected_metadata_path "\$PPRDIR/${project_name}.xpr"
if {$project_metadata_path ne $expected_metadata_path} {
    error "Project Path metadata is not portable: $project_metadata_path"
}

open_project $project_path
if {[get_property PART [current_project]] ne "xc7z020clg400-2"} {
    error "Wrong FPGA part: [get_property PART [current_project]]"
}

set expected_top [expr {$profile eq "selftest" ?
    "aud_bx71_selftest_top" : "audio_soc_wrapper"}]
if {[get_property TOP [get_filesets sources_1]] ne $expected_top} {
    error "Wrong top for $profile: [get_property TOP [get_filesets sources_1]]"
}

if {$profile eq "soc_i2s"} {
    set bd_files [get_files -quiet */audio_soc.bd]
    if {[llength $bd_files] != 1} {
        error "Expected one audio_soc.bd, found [llength $bd_files]"
    }
    open_bd_design [lindex $bd_files 0]
    validate_bd_design
    # Clean clones contain native .bd/.xpr and RTL, not generated Xilinx IP.
    generate_target all $bd_files
}

set missing {}
set external {}
set normalized_fpga_dir [string map {\\ /} $fpga_dir]
foreach source [get_files -all] {
    set source_path [file normalize $source]
    if {![file exists $source_path]} {
        lappend missing $source_path
    }
    set normalized_source [string map {\\ /} $source_path]
    if {![string match "${normalized_fpga_dir}/*" $normalized_source]} {
        lappend external $source_path
    }
}
if {[llength $missing] != 0} {
    error "Project has missing sources: $missing"
}
if {[llength $external] != 0} {
    error "Project has sources outside fpga/: $external"
}

update_compile_order -fileset sources_1
update_compile_order -fileset sim_1
puts "FPGA_NATIVE_PROJECT_OK profile=$profile part=[get_property PART [current_project]] top=$expected_top files=[llength [get_files -all]]"
close_project
exit 0
