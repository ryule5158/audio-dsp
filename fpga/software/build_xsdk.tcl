set software_dir [file normalize [file dirname [info script]]]
set fpga_dir [file normalize [file join $software_dir ..]]
set profile [expr {$argc > 0 ? [lindex $argv 0] : "audio"}]
if {$profile ne "audio"} { error "Only the audio software profile is supported" }
set app_name wm8960_demo
set workspace_name xsdk
set hdf [file join $fpga_dir build output audio_dsp_bx71_soc_i2s.hdf]
set source_dir [file join $software_dir baremetal src]
set workspace [file join $fpga_dir build $workspace_name]
set output_dir [file join $fpga_dir build output]
set output_elf [file join $output_dir audio_dsp_bx71_${app_name}.elf]

if {![file exists $hdf]} {
    error "Missing HDF: $hdf. Run .\\build.ps1 soc_i2s all first."
}
if {![file isdirectory $source_dir]} {
    error "Missing source directory: $source_dir"
}
if {![info exists ::env(AUDIO_DSP_XSDK_MAKE)] ||
    ![file exists $::env(AUDIO_DSP_XSDK_MAKE)]} {
    error "AUDIO_DSP_XSDK_MAKE must name the XSDK 2018.3 make.exe"
}
set make_exe [file normalize $::env(AUDIO_DSP_XSDK_MAKE)]
if {![info exists ::env(AUDIO_DSP_XSDK_READELF)] ||
    ![file exists $::env(AUDIO_DSP_XSDK_READELF)]} {
    error "AUDIO_DSP_XSDK_READELF must name the XSDK 2018.3 readelf.exe"
}
set readelf_exe [file normalize $::env(AUDIO_DSP_XSDK_READELF)]

# Only the generated audio workspace is replaced. Never use a wildcard parent
# as a cleanup target; unrelated generated experiments are untouched.
set normalized_workspace [string map {\\ /} [file normalize $workspace]]
set expected_workspace [string map {\\ /} [file join $fpga_dir build $workspace_name]]
if {$normalized_workspace ne $expected_workspace ||
    $workspace_name ne "xsdk"} {
    error "Refusing to clean unexpected XSDK workspace: $workspace"
}
if {[file exists $workspace]} {
    file delete -force $workspace
}
file mkdir $workspace
file mkdir $output_dir
# Never leave an obsolete DDR-linked ELF looking like the result of a failed
# OCM rebuild.  This path is fixed below fpga/build/output.
if {[file exists $output_elf]} {
    file delete -force $output_elf
}

setws $workspace
createhw -name audio_hw -hwspec $hdf
createbsp -name audio_bsp -hwproject audio_hw -proc ps7_cortexa9_0 -os standalone
configbsp -bsp audio_bsp stdin ps7_uart_1
configbsp -bsp audio_bsp stdout ps7_uart_1
regenbsp -bsp audio_bsp
createapp -name $app_name -app {Empty Application} -hwproject audio_hw \
    -proc ps7_cortexa9_0 -os standalone -lang c -bsp audio_bsp
importsources -name $app_name -path $source_dir

# Some Windows XSDK 2018.3 installations generate managed makefiles but the
# Eclipse launcher cannot resolve bare `make` in headless mode. Invoke the same
# generated projects with Xilinx's bundled make by absolute path and treat any
# nonzero status as a hard failure.
set bsp_makefile [file join $workspace audio_bsp Makefile]
set app_makefile [file join $workspace $app_name Debug makefile]
if {![file exists $bsp_makefile]} {
    error "XSDK did not generate the expected BSP Makefile"
}
set linker_script [file join $workspace $app_name src lscript.ld]
if {![file exists $linker_script]} {
    error "XSDK did not generate lscript.ld"
}
# PCW_EN_DDR=0 is intentional, but the 2018.3 linker-script generator still
# selects the PS DDR range. Retarget every generated section to low OCM and
# leave the unused memory declarations untouched for hardware traceability.
set linker_handle [open $linker_script r]
set linker_text [read $linker_handle]
close $linker_handle
if {[string first "> ps7_ddr_0" $linker_text] < 0} {
    error "Unexpected linker script: no generated ps7_ddr_0 mappings"
}
set linker_text [string map {"> ps7_ddr_0" "> ps7_ram_0"} $linker_text]
set linker_handle [open $linker_script w]
puts -nonewline $linker_handle $linker_text
close $linker_handle
set generated_spec [file join $workspace $app_name src Xilinx.spec]
set build_spec [file join $workspace $app_name Debug Xilinx.spec]
if {![file exists $generated_spec]} {
    error "XSDK did not generate Xilinx.spec"
}
# Equivalent to XSDK's a9-linaro-pre-build-step, whose extensionless helper is
# not launched reliably by GNU make on Windows when running headless.
file mkdir [file dirname $build_spec]
file copy -force $generated_spec $build_spec
puts [exec $make_exe -C [file dirname $bsp_makefile] all 2>@1]
# CDT creates Debug/makefile only when its managed builder is invoked, not
# during createapp/importsources. OCM mappings and Xilinx.spec are in place
# before that first build. Preserve any launcher failure message; the same
# generated makefile must subsequently pass a direct make and ELF range gate.
if {[catch {projects -build -type app -name $app_name} managed_message]} {
    puts "FPGA_XSDK_MANAGED_BUILD_NOTE: $managed_message"
}
if {![file exists $app_makefile]} {
    error "XSDK managed builder did not generate the application makefile"
}
puts [exec $make_exe -C [file dirname $app_makefile] all 2>@1]

set elf_candidates [glob -nocomplain \
    [file join $workspace $app_name Debug ${app_name}.elf] \
    [file join $workspace $app_name Release ${app_name}.elf]]
if {[llength $elf_candidates] != 1} {
    error "Expected one ${app_name}.elf, found: $elf_candidates"
}
set elf [lindex $elf_candidates 0]
set elf_file_header [exec $readelf_exe -h $elf 2>@1]
if {![regexp {Entry point address:\s+(0x[0-9a-fA-F]+)} \
        $elf_file_header unused entry_address]} {
    error "Cannot parse ELF entry point:\n$elf_file_header"
}
scan $entry_address %x entry_value
if {$entry_value < 0 || $entry_value >= 0x00030000} {
    error "ELF entry point escapes low OCM: $entry_address"
}
set elf_program_headers [exec $readelf_exe -l $elf 2>@1]
set load_segment_count 0
set entry_in_load 0
set load_ranges {}
foreach line [split $elf_program_headers "\n"] {
    if {[regexp {^\s*LOAD\s+\S+\s+(0x[0-9a-fA-F]+)\s+(0x[0-9a-fA-F]+)\s+\S+\s+(0x[0-9a-fA-F]+)} \
            $line unused virtual_address physical_address memory_size]} {
        scan $virtual_address %x virtual_value
        scan $physical_address %x physical_value
        scan $memory_size %x memory_value
        incr load_segment_count
        if {$virtual_value != $physical_value || $virtual_value < 0 ||
            ($virtual_value + $memory_value) > 0x00030000} {
            error "ELF LOAD segment escapes low OCM: $line"
        }
        set segment_end [expr {$virtual_value + $memory_value}]
        if {$entry_value >= $virtual_value && $entry_value < $segment_end} {
            set entry_in_load 1
        }
        lappend load_ranges [format "0x%08X..0x%08X" \
            $virtual_value $segment_end]
    }
}
if {$load_segment_count == 0} {
    error "ELF has no LOAD segments:\n$elf_program_headers"
}
if {!$entry_in_load} {
    error "ELF entry point $entry_address is not covered by a LOAD segment"
}
file copy -force $elf $output_elf
if {[file size $output_elf] == 0} {
    error "Generated ELF is empty: $output_elf"
}

puts "FPGA_XSDK_BUILD_OK profile=$profile elf=$output_elf bytes=[file size $output_elf] hdf=$hdf stdout=ps7_uart_1 memory=OCM entry=[format 0x%08X $entry_value] load_segments=$load_segment_count load_ranges=[join $load_ranges ,]"
exit
