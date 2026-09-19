# Materialize the two checked-in Vivado 2018.3 projects.  These projects are
# first-class GUI entry points; flow.tcl remains the reproducible clean-build
# entry point used for release verification.

set script_dir [file dirname [file normalize [info script]]]
set fpga_dir [file normalize [file join $script_dir ..]]
set projects_dir [file join $fpga_dir projects]

proc make_xpr_path_portable {project_file project_name} {
    set handle [open $project_file r]
    set project_text [read $handle]
    close $handle

    set absolute_path [string map {\\ /} [file normalize $project_file]]
    set absolute_token "Path=\"$absolute_path\""
    set portable_token "Path=\"\$PPRDIR/${project_name}.xpr\""
    if {[string first $absolute_token $project_text] < 0} {
        error "Cannot find generated absolute project path in $project_file"
    }
    set project_text [string map [list $absolute_token $portable_token] \
        $project_text]

    set handle [open $project_file w]
    puts -nonewline $handle $project_text
    close $handle
}

foreach profile {selftest soc_i2s} {
    set project_name audio_dsp_bx71_${profile}
    set argv [list $profile $projects_dir]
    set argc 2
    source [file join $script_dir create_project.tcl]
    # Vivado 2018.3 persists project-mode changes as they are applied. Its
    # save_project alias requires save_project_as arguments, not a bare call.
    close_project
    set project_file [file join $projects_dir $project_name \
        ${project_name}.xpr]
    make_xpr_path_portable $project_file $project_name
    puts "FPGA_NATIVE_PROJECT_CREATED profile=$profile"
}

puts "FPGA_NATIVE_PROJECTS_OK root=$projects_dir"
exit 0
