param(
    [string]$ProjectPath = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Join-Path $PSScriptRoot `
        "..\project\STM32H743_Audio\MDK-ARM\STM32H743_Audio.uvprojx"
}
$ProjectPath = (Resolve-Path -LiteralPath $ProjectPath).Path
$OptionsPath = [System.IO.Path]::ChangeExtension($ProjectPath, ".uvoptx")
if (-not (Test-Path -LiteralPath $OptionsPath)) {
    throw "CubeMX generated no Keil user-options file: $OptionsPath"
}
$mdkDir = Split-Path -Parent $ProjectPath

function Set-XmlChildText {
    param(
        [System.Xml.XmlElement]$Parent,
        [string]$Name,
        [string]$Value
    )

    $node = $Parent.SelectSingleNode($Name)
    if ($null -eq $node) {
        $node = $Parent.OwnerDocument.CreateElement($Name)
        [void]$Parent.AppendChild($node)
    }
    $node.InnerText = $Value
}

function New-SourceFileNode {
    param(
        [xml]$Document,
        [string]$Name,
        [string]$Path
    )

    $file = $Document.CreateElement("File")
    foreach ($item in @(
        @("FileName", $Name),
        @("FileType", "1"),
        @("FilePath", $Path)
    )) {
        $child = $Document.CreateElement($item[0])
        $child.InnerText = $item[1]
        [void]$file.AppendChild($child)
    }
    return $file
}

function Set-FileIncludeInBuild {
    param(
        [System.Xml.XmlElement]$File,
        [System.Xml.XmlElement]$Template,
        [int]$IncludeInBuild
    )

    $fileOption = $File.SelectSingleNode("FileOption")
    if ($null -eq $fileOption) {
        if ($null -eq $Template) {
            throw "Cannot add a file-level IncludeInBuild property without a template"
        }
        $fileOption = $Template.CloneNode($true)
        [void]$File.AppendChild($fileOption)
    }
    $includeNode = $fileOption.SelectSingleNode(
        "CommonProperty/IncludeInBuild")
    if ($null -eq $includeNode) {
        throw "Keil FileOption template has no CommonProperty/IncludeInBuild"
    }
    $includeNode.InnerText = [string]$IncludeInBuild
}

function Set-FileCompilerFlags {
    param(
        [System.Xml.XmlElement]$File,
        [string]$Flags
    )

    $cads = $File.SelectSingleNode("FileOption/FileArmAds/Cads")
    if ($null -eq $cads) {
        throw "Keil C-file option has no FileArmAds/Cads node"
    }
    $controls = $cads.SelectSingleNode("VariousControls")
    if ($null -eq $controls) {
        $controls = $File.OwnerDocument.CreateElement("VariousControls")
        foreach ($name in @("MiscControls", "Define", "Undefine", "IncludePath")) {
            [void]$controls.AppendChild($File.OwnerDocument.CreateElement($name))
        }
        [void]$cads.AppendChild($controls)
    }
    Set-XmlChildText $controls "MiscControls" $Flags
}

# Keep the Keil project in lock-step with the portable CMake manifest.  These
# are the 49 MIT/permissive DaisySP-derived translation units; the 17
# LGPL-2.1-only units are intentionally an explicit, separate opt-in and are
# not silently pulled into the reference firmware.
$permissiveSources = @(
    "aud_adenv.c", "aud_adsr.c", "aud_analogbassdrum.c",
    "aud_analogsnaredrum.c", "aud_autowah.c", "aud_chorus.c",
    "aud_clockednoise.c", "aud_crossfade.c", "aud_dcblock.c",
    "aud_decimator.c", "aud_drip.c", "aud_dust.c", "aud_flanger.c",
    "aud_fm2.c", "aud_formantosc.c", "aud_fractal_noise.c",
    "aud_grainlet.c", "aud_granularplayer.c", "aud_harmonic_osc.c",
    "aud_hihat.c", "aud_karplusstring.c", "aud_limiter.c",
    "aud_looper.c", "aud_maytrig.c", "aud_metro.c", "aud_modalvoice.c",
    "aud_osc.c", "aud_oscillatorbank.c", "aud_overdrive.c",
    "aud_particle.c", "aud_phaser.c", "aud_phasor.c",
    "aud_pitchshifter.c", "aud_resonator.c", "aud_samplehold.c",
    "aud_sampleratereducer.c", "aud_smooth_random.c", "aud_soap.c",
    "aud_stringvoice.c", "aud_svf.c", "aud_synthbassdrum.c",
    "aud_synthsnaredrum.c", "aud_tremolo.c", "aud_variablesawosc.c",
    "aud_variableshapeosc.c", "aud_vosim.c", "aud_wavefolder.c",
    "aud_whitenoise.c", "aud_zoscillator.c"
)
if ($permissiveSources.Count -ne 49) {
    throw "Internal permissive DSP manifest must contain 49 source files"
}
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..\..")).Path
foreach ($source in $permissiveSources) {
    if (-not (Test-Path -LiteralPath (Join-Path $repoRoot $source))) {
        throw "Portable DSP source is missing: $source"
    }
}

# User-provided H743/DSP is kept as one isolated, opt-in analysis library.  Keil
# stores the project file tree as a synchronized multi-target model, so the
# group is present in every target and its C files use file-level
# IncludeInBuild flags to remain opt-in only for the analysis image.
$genericDspSources = @(
    "Adaptive.c", "Correlate.c", "Demod.c", "DSP_ProMax.c", "FFT.c",
    "Filter.c", "FilterEx.c", "Fit.c", "IQ.c", "Measure.c", "ModelFit.c",
    "periodic_analyzer.c", "SoftPll.c"
)
$genericDspHeaders = @(
    "Adaptive.h", "Correlate.h", "Demod.h", "DSP_Header.h",
    "DSP_ProMax.h", "FFT.h", "Filter.h", "FilterEx.h", "Fit.h", "IQ.h",
    "Measure.h", "ModelFit.h", "periodic_analyzer.h", "SoftPll.h"
)
$genericDspDir = Join-Path $repoRoot "mcu\project\STM32H743_Audio\App\Generic_DSP"
foreach ($source in ($genericDspSources + $genericDspHeaders +
        @("generic_dsp_selftest.c", "generic_dsp_selftest.h"))) {
    $sourcePath = if ($source -like "generic_dsp_*") {
        Join-Path $repoRoot "mcu\project\STM32H743_Audio\App\Src\$source"
    } else {
        Join-Path $genericDspDir $source
    }
    if (-not (Test-Path -LiteralPath $sourcePath)) {
        throw "Generic DSP source/header is missing: $sourcePath"
    }
}

function New-AnalysisGroup {
    param(
        [xml]$Document,
        [System.Xml.XmlElement]$FileOptionTemplate,
        [int]$IncludeInBuild
    )

    $analysisGroup = $Document.CreateElement("Group")
    $analysisName = $Document.CreateElement("GroupName")
    $analysisName.InnerText = "Library/Generic DSP"
    [void]$analysisGroup.AppendChild($analysisName)
    $analysisFiles = $Document.CreateElement("Files")
    foreach ($source in $genericDspSources) {
        $sourceNode = New-SourceFileNode $Document $source `
            ("../App/Generic_DSP/" + $source)
        Set-FileIncludeInBuild -File $sourceNode `
            -Template $FileOptionTemplate -IncludeInBuild $IncludeInBuild
        [void]$analysisFiles.AppendChild($sourceNode)
    }
    $adapterNode = New-SourceFileNode $Document `
        "generic_dsp_selftest.c" "../App/Src/generic_dsp_selftest.c"
    Set-FileIncludeInBuild -File $adapterNode `
        -Template $FileOptionTemplate -IncludeInBuild $IncludeInBuild
    [void]$analysisFiles.AppendChild($adapterNode)
    foreach ($header in $genericDspHeaders) {
        $headerNode = New-SourceFileNode $Document $header `
            ("../App/Generic_DSP/" + $header)
        $headerNode.SelectSingleNode("FileType").InnerText = "5"
        [void]$analysisFiles.AppendChild($headerNode)
    }
    $adapterHeader = New-SourceFileNode $Document `
        "generic_dsp_selftest.h" "../App/Src/generic_dsp_selftest.h"
    $adapterHeader.SelectSingleNode("FileType").InnerText = "5"
    [void]$analysisFiles.AppendChild($adapterHeader)
    [void]$analysisGroup.AppendChild($analysisFiles)
    return $analysisGroup
}

# LVGL is imported from the official v9.5.0 source tree.  Keep the complete C
# source manifest synchronized in every Keil target, but use file-level
# IncludeInBuild flags so only the optional LVGL_UI image compiles it.
$lvglSourceRoot = Join-Path $repoRoot "mcu\project\STM32H743_Audio\App\LVGL\src"
$lvglSources = @()
if (-not (Test-Path -LiteralPath $lvglSourceRoot)) {
    throw "Official LVGL source tree is missing: $lvglSourceRoot"
}
foreach ($sourcePath in @(Get-ChildItem -LiteralPath $lvglSourceRoot -Recurse -File -Filter "*.c" |
        Sort-Object FullName)) {
    $relative = $sourcePath.FullName.Substring($lvglSourceRoot.Length + 1) `
        -replace '\\', '/'
    $lvglSources += [pscustomobject]@{
        Name = $sourcePath.Name
        Path = "../App/LVGL/src/" + $relative
        Absolute = $sourcePath.FullName
    }
}
$lvglSources += [pscustomobject]@{
    Name = "lvgl_port_stm32.c"
    Path = "../App/LVGL_UI/lvgl_port_stm32.c"
    Absolute = (Join-Path $repoRoot "mcu\project\STM32H743_Audio\App\LVGL_UI\lvgl_port_stm32.c")
}
$lvglSources += [pscustomobject]@{
    Name = "lvgl_ui.c"
    Path = "../App/LVGL_UI/lvgl_ui.c"
    Absolute = (Join-Path $repoRoot "mcu\project\STM32H743_Audio\App\LVGL_UI\lvgl_ui.c")
}
$lvglSources += [pscustomobject]@{
    Name = "stm32h7xx_hal_spi.c"
    Path = "../Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c"
    Absolute = (Join-Path $repoRoot "mcu\project\STM32H743_Audio\Drivers\STM32H7xx_HAL_Driver\Src\stm32h7xx_hal_spi.c")
}
foreach ($source in $lvglSources) {
    if (-not (Test-Path -LiteralPath $source.Absolute)) {
        throw "LVGL source is missing: $($source.Absolute)"
    }
}

function New-LvglGroup {
    param(
        [xml]$Document,
        [System.Xml.XmlElement]$FileOptionTemplate,
        [int]$IncludeInBuild
    )

    $lvglGroup = $Document.CreateElement("Group")
    $lvglName = $Document.CreateElement("GroupName")
    $lvglName.InnerText = "Library/LVGL v9.5"
    [void]$lvglGroup.AppendChild($lvglName)
    $lvglFiles = $Document.CreateElement("Files")
    foreach ($source in $lvglSources) {
        $sourceNode = New-SourceFileNode $Document $source.Name $source.Path
        Set-FileIncludeInBuild -File $sourceNode -Template $FileOptionTemplate `
            -IncludeInBuild $IncludeInBuild
        if ($IncludeInBuild -eq 1) {
            if ($source.Path -match "/lv_draw_sw_blend_to_rgb565(?:_swapped)?\.c$") {
                Set-FileCompilerFlags -File $sourceNode `
                    -Flags "-Wno-unused-function"
            }
            elseif ($source.Path -match "/lv_tlsf\.c$") {
                Set-FileCompilerFlags -File $sourceNode `
                    -Flags "-Wno-unused-parameter"
            }
        }
        [void]$lvglFiles.AppendChild($sourceNode)
    }
    [void]$lvglGroup.AppendChild($lvglFiles)
    return $lvglGroup
}

[xml]$document = Get-Content -Raw -LiteralPath $ProjectPath
$target = $document.SelectSingleNode("/Project/Targets/Target")
if ($null -eq $target) {
    throw "Keil target is missing from $ProjectPath"
}

Set-XmlChildText $target "pArmCC" "6240000::V6.24::ARMCLANG"
Set-XmlChildText $target "pCCUsed" "6240000::V6.24::ARMCLANG"
Set-XmlChildText $target "uAC6" "1"

$common = $target.SelectSingleNode("TargetOption/TargetCommonOption")
$cads = $target.SelectSingleNode(
    "TargetOption/TargetArmAds/Cads/VariousControls")
$ldads = $target.SelectSingleNode("TargetOption/TargetArmAds/LDads")
$groups = $target.SelectSingleNode("Groups")
if (($null -eq $common) -or ($null -eq $cads) -or
    ($null -eq $ldads) -or ($null -eq $groups)) {
    throw "CubeMX generated an unexpected Keil project structure"
}

Set-XmlChildText $common "PackID" "Keil.STM32H7xx_DFP.4.1.3"

$includePath = $cads.SelectSingleNode("IncludePath")
$includes = @($includePath.InnerText -split ";" |
    Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
if ($includes -notcontains "../App/Inc") {
    $includes += "../App/Inc"
}
$rootInclude = "../../../.."
if ($includes -notcontains $rootInclude) {
    $includes += $rootInclude
}
$genericDspInclude = "../App/Generic_DSP"
$lvglIncludePaths = @("../App/LVGL", "../App/LVGL/src", "../App/LVGL_UI")
$includes = @($includes | Where-Object {
    $_ -ne $genericDspInclude -and $_ -notin $lvglIncludePaths
})
$includePath.InnerText = $includes -join ";"

Set-XmlChildText $ldads "umfTarg" "0"
Set-XmlChildText $ldads "useFile" "0"
Set-XmlChildText $ldads "ScatterFile" `
    "../App/Linker/STM32H743_Audio.sct"

foreach ($file in @($document.SelectNodes(
    "//File[not(FilePath) or normalize-space(FilePath)='']"))) {
    [void]$file.ParentNode.RemoveChild($file)
}
foreach ($group in @($groups.SelectNodes(
    "Group[GroupName='Application/Audio' or GroupName='Library/DSP permissive' or GroupName='Library/Generic DSP' or GroupName='Library/LVGL v9.5']"))) {
    [void]$groups.RemoveChild($group)
}

$audioGroup = $document.CreateElement("Group")
$groupName = $document.CreateElement("GroupName")
$groupName.InnerText = "Application/Audio"
[void]$audioGroup.AppendChild($groupName)
$files = $document.CreateElement("Files")
[void]$files.AppendChild((New-SourceFileNode $document `
    "audio_app.c" "../App/Src/audio_app.c"))
[void]$files.AppendChild((New-SourceFileNode $document `
    "audio_dsp.c" "../App/Src/audio_dsp.c"))
[void]$files.AppendChild((New-SourceFileNode $document `
    "wm8960.c" "../App/Src/wm8960.c"))
[void]$audioGroup.AppendChild($files)
[void]$groups.AppendChild($audioGroup)

$dspGroup = $document.CreateElement("Group")
$dspGroupName = $document.CreateElement("GroupName")
$dspGroupName.InnerText = "Library/DSP permissive"
[void]$dspGroup.AppendChild($dspGroupName)
$dspFiles = $document.CreateElement("Files")
foreach ($source in $permissiveSources) {
    [void]$dspFiles.AppendChild((New-SourceFileNode $document $source `
        ("../../../../" + $source)))
}
$header = New-SourceFileNode $document "aud_daisysp_permissive.h" `
    "../../../../aud_daisysp_permissive.h"
$header.SelectSingleNode("FileType").InnerText = "5"
[void]$dspFiles.AppendChild($header)
[void]$dspGroup.AppendChild($dspFiles)
[void]$groups.AppendChild($dspGroup)

foreach ($component in @($document.SelectNodes(
    "/Project/RTE/components/component[@Cclass='CMSIS' and @Cgroup='CORE']"))) {
    [void]$component.ParentNode.RemoveChild($component)
}

# Keep Pack metadata explicit and target-scoped.  Keil resolves the installed
# CMSIS 6.3.0/CMSIS-DSP 1.16.2 packs only for the analysis target, so the
# existing two targets remain independent of the optional Pack source tree.
$rte = $document.SelectSingleNode("/Project/RTE")
if ($null -eq $rte) {
    $rte = $document.CreateElement("RTE")
    [void]$document.Project.AppendChild($rte)
}
foreach ($childName in @("apis", "components", "files")) {
    if ($null -eq $rte.SelectSingleNode($childName)) {
        [void]$rte.AppendChild($document.CreateElement($childName))
    }
}
$rteComponents = $rte.SelectSingleNode("components")
foreach ($component in @($rteComponents.SelectNodes(
    "component[@Cclass='CMSIS' and (@Cgroup='CORE' or @Cgroup='DSP')]"))) {
    [void]$rteComponents.RemoveChild($component)
}
function New-RteComponent {
    param(
        [xml]$Document,
        [System.Xml.XmlElement]$Parent,
        [hashtable]$Attributes,
        [hashtable]$PackageAttributes,
        [string]$TargetName
    )
    $component = $Document.CreateElement("component")
    foreach ($key in $Attributes.Keys) {
        $component.SetAttribute($key, [string]$Attributes[$key])
    }
    $packageNode = $Document.CreateElement("package")
    foreach ($key in $PackageAttributes.Keys) {
        $packageNode.SetAttribute($key, [string]$PackageAttributes[$key])
    }
    [void]$component.AppendChild($packageNode)
    $targetInfos = $Document.CreateElement("targetInfos")
    $targetInfo = $Document.CreateElement("targetInfo")
    $targetInfo.SetAttribute("name", $TargetName)
    [void]$targetInfos.AppendChild($targetInfo)
    [void]$component.AppendChild($targetInfos)
    [void]$Parent.AppendChild($component)
}
New-RteComponent $document $rteComponents `
    @{ Cclass="CMSIS"; Cgroup="CORE"; Cvendor="ARM"; Cversion="6.2.0";
       condition="ARMv6_7_8-M Device"; ymlID="CMSIS:CORE" } `
    @{ name="CMSIS"; schemaVersion="1.7.53";
       url="https://www.keil.com/pack/"; vendor="ARM"; version="6.3.0" } `
    "STM32H743_Audio_Generic_DSP"
New-RteComponent $document $rteComponents `
    @{ Cclass="CMSIS"; Cgroup="DSP"; Cvariant="Source"; Cvendor="ARM";
       Cversion="1.16.2"; condition="CMSISCORE"; ymlID="CMSIS:DSP&Source" } `
    @{ name="CMSIS-DSP"; schemaVersion="1.7.27";
       url="https://www.keil.com/pack/"; vendor="ARM"; version="1.16.2" } `
    "STM32H743_Audio_Generic_DSP"

# CubeMX emits one target.  Keep four explicit, reproducible variants so a
# developer can build the safe bench self-test, the real WM8960 stream, the
# opt-in Generic DSP analysis image, or the isolated LVGL UI image without
# editing a header by hand.
$targets = $document.SelectSingleNode("/Project/Targets")
$baseTarget = $targets.SelectSingleNode("Target")
if ($null -eq $baseTarget) {
    throw "CubeMX generated no Keil target"
}
$analysisFileOptionTemplate = $baseTarget.SelectSingleNode(
    "Groups/Group/Files/File[FileType='1' and FileOption]/FileOption")
if ($null -eq $analysisFileOptionTemplate -or
    $null -eq $analysisFileOptionTemplate.SelectSingleNode(
        "CommonProperty/IncludeInBuild")) {
    throw "CubeMX target has no complete C-file option template for Generic_DSP files"
}
$targetCopies = @()
foreach ($spec in @(
    @("STM32H743_Audio_SelfTest", "STM32H743_Audio_SelfTest\", "AUDIO_BOARD_ENABLE_WM8960_STREAM=0", "0", "0"),
    @("STM32H743_Audio_WM8960_Stream", "STM32H743_Audio_WM8960_Stream\", "AUDIO_BOARD_ENABLE_WM8960_STREAM=1", "0", "0"),
    @("STM32H743_Audio_Generic_DSP", "STM32H743_Audio_Generic_DSP\", "AUDIO_BOARD_ENABLE_WM8960_STREAM=0", "1", "0"),
    @("STM32H743_Audio_LVGL_UI", "STM32H743_Audio_LVGL_UI\", "AUDIO_BOARD_ENABLE_WM8960_STREAM=0", "0", "1")
)) {
    $copy = $baseTarget.CloneNode($true)
    $copy.SelectSingleNode("TargetName").InnerText = $spec[0]
    $copy.SelectSingleNode("TargetOption/TargetCommonOption/OutputDirectory").InnerText = $spec[1]
    $copy.SelectSingleNode("TargetOption/TargetCommonOption/OutputName").InnerText = $spec[0]
    $copy.SelectSingleNode("TargetOption/TargetCommonOption/ListingPath").InnerText = "./" + $spec[0] + "/"
    $define = $copy.SelectSingleNode("TargetOption/TargetArmAds/Cads/VariousControls/Define")
    $baseDefines = @($define.InnerText -split "," |
        Where-Object {
            ($_ -notmatch "^AUDIO_BOARD_ENABLE_WM8960_STREAM=") -and
            ($_ -notmatch "^AUDIO_BOARD_ENABLE_GENERIC_DSP=") -and
            ($_ -notmatch "^AUDIO_BOARD_ENABLE_LVGL_UI=") -and
            ($_ -notmatch "^HAL_SPI_MODULE_ENABLED$") -and
            ($_ -notmatch "^LV_CONF_INCLUDE_SIMPLE$")
        })
    $extraDefines = @(
        $spec[2],
        "AUDIO_BOARD_ENABLE_GENERIC_DSP=$($spec[3])",
        "AUDIO_BOARD_ENABLE_LVGL_UI=$($spec[4])"
    )
    if ($spec[4] -eq "1") {
        $extraDefines += @("HAL_SPI_MODULE_ENABLED", "LV_CONF_INCLUDE_SIMPLE")
    }
    $define.InnerText = (($baseDefines + $extraDefines) -join ",")
    if ($spec[3] -eq "1") {
        $copyIncludePath = $copy.SelectSingleNode(
            "TargetOption/TargetArmAds/Cads/VariousControls/IncludePath")
        if ($null -eq $copyIncludePath) {
            throw "Generic DSP target has no IncludePath node"
        }
        $copyIncludes = @($copyIncludePath.InnerText -split ";" |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
        if ($copyIncludes -notcontains $genericDspInclude) {
            $copyIncludes += $genericDspInclude
        }
        $copyIncludePath.InnerText = $copyIncludes -join ";"
    }
    if ($spec[4] -eq "1") {
        $copyIncludePath = $copy.SelectSingleNode(
            "TargetOption/TargetArmAds/Cads/VariousControls/IncludePath")
        if ($null -eq $copyIncludePath) {
            throw "LVGL target has no IncludePath node"
        }
        $copyIncludes = @($copyIncludePath.InnerText -split ";" |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
        foreach ($lvglInclude in $lvglIncludePaths) {
            if ($copyIncludes -notcontains $lvglInclude) {
                $copyIncludes += $lvglInclude
            }
        }
        $copyIncludePath.InnerText = $copyIncludes -join ";"
    }
    $copyGroups = $copy.SelectSingleNode("Groups")
    if ($null -eq $copyGroups) {
        throw "Keil target has no Groups node"
    }
    [void]$copyGroups.AppendChild((New-AnalysisGroup $document `
        $analysisFileOptionTemplate ([int]$spec[3])))
    [void]$copyGroups.AppendChild((New-LvglGroup $document `
        $analysisFileOptionTemplate ([int]$spec[4])))
    $targetCopies += $copy
}
foreach ($oldTarget in @($targets.SelectNodes("Target"))) {
    [void]$targets.RemoveChild($oldTarget)
}
foreach ($copy in $targetCopies) {
    [void]$targets.AppendChild($copy)
}

# Keep .uvoptx aligned as well.  Leaving CubeMX's obsolete single target in
# this file makes the project appear broken when opened interactively even
# though command-line builds address the .uvprojx targets directly.
[xml]$options = Get-Content -Raw -LiteralPath $OptionsPath
$optionBase = $options.SelectSingleNode("/ProjectOpt/Target")
if ($null -eq $optionBase) {
    throw "CubeMX generated an unexpected Keil user-options structure"
}
$optionTargets = @()
foreach ($spec in @(
    @("STM32H743_Audio_SelfTest", "1"),
    @("STM32H743_Audio_WM8960_Stream", "0"),
    @("STM32H743_Audio_Generic_DSP", "0"),
    @("STM32H743_Audio_LVGL_UI", "0")
)) {
    $copy = $optionBase.CloneNode($true)
    $copy.SelectSingleNode("TargetName").InnerText = $spec[0]
    $copy.SelectSingleNode(
        "TargetOption/OPTFL/IsCurrentTarget").InnerText = $spec[1]
    $listing = $copy.SelectSingleNode("TargetOption/OPTLEX/ListingPath")
    if ($null -ne $listing) {
        $listing.InnerText = "./" + $spec[0] + "/"
    }
    $optionTargets += $copy
}
$optionsRoot = $options.SelectSingleNode("/ProjectOpt")
foreach ($oldTarget in @($options.SelectNodes("/ProjectOpt/Target"))) {
    [void]$optionsRoot.RemoveChild($oldTarget)
}
foreach ($copy in $optionTargets) {
    [void]$optionsRoot.AppendChild($copy)
}

# CubeMX names debugger/RTE side files after its temporary one-target project.
# Materialize debugger configuration for all final targets and remove the
# obsolete base-target files so opening the project never exposes a dead name.
$debugDir = Join-Path $mdkDir "DebugConfig"
$baseDebug = Join-Path $debugDir `
    "STM32H743_Audio_STM32H743IITx_1.1.1.dbgconf"
$debugSource = $null
if (Test-Path -LiteralPath $baseDebug) {
    $debugSource = $baseDebug
} elseif (Test-Path -LiteralPath $debugDir) {
    $debugSource = @(
        "STM32H743_Audio_SelfTest_STM32H743IITx_1.1.1.dbgconf",
        "STM32H743_Audio_WM8960_Stream_STM32H743IITx_1.1.1.dbgconf",
        "STM32H743_Audio_Generic_DSP_STM32H743IITx_1.1.1.dbgconf",
        "STM32H743_Audio_LVGL_UI_STM32H743IITx_1.1.1.dbgconf"
    ) | ForEach-Object { Join-Path $debugDir $_ } |
        Where-Object { Test-Path -LiteralPath $_ } |
        Select-Object -First 1
}
if ($null -ne $debugSource) {
    foreach ($targetName in @("STM32H743_Audio_SelfTest",
                               "STM32H743_Audio_WM8960_Stream",
                               "STM32H743_Audio_Generic_DSP",
                               "STM32H743_Audio_LVGL_UI")) {
        $debugDestination = Join-Path $debugDir `
            ($targetName + "_STM32H743IITx_1.1.1.dbgconf")
        if (-not $debugSource.Equals($debugDestination,
            [System.StringComparison]::OrdinalIgnoreCase)) {
            Copy-Item -LiteralPath $debugSource -Destination $debugDestination -Force
        }
    }
    if (Test-Path -LiteralPath $baseDebug) {
        Remove-Item -LiteralPath $baseDebug -Force
    }
}
$obsoleteRte = Join-Path $mdkDir "RTE\_STM32H743_Audio"
if (Test-Path -LiteralPath $obsoleteRte) {
    $resolvedObsolete = (Resolve-Path -LiteralPath $obsoleteRte).Path
    $rtePrefix = [IO.Path]::GetFullPath((Join-Path $mdkDir "RTE")) +
        [IO.Path]::DirectorySeparatorChar
    if (-not $resolvedObsolete.StartsWith($rtePrefix,
            [StringComparison]::OrdinalIgnoreCase) -or
        (((Get-Item -LiteralPath $resolvedObsolete).Attributes -band
            [IO.FileAttributes]::ReparsePoint) -ne 0)) {
        throw "Refusing unexpected obsolete RTE path: $resolvedObsolete"
    }
    Remove-Item -LiteralPath $resolvedObsolete -Recurse -Force
}

$settings = New-Object System.Xml.XmlWriterSettings
$settings.Indent = $true
$settings.Encoding = New-Object System.Text.UTF8Encoding($false)
$writer = [System.Xml.XmlWriter]::Create($ProjectPath, $settings)
try {
    $document.Save($writer)
} finally {
    $writer.Dispose()
}

$optionsWriter = [System.Xml.XmlWriter]::Create($OptionsPath, $settings)
try {
    $options.Save($optionsWriter)
} finally {
    $optionsWriter.Dispose()
}

[xml]$projectCheck = Get-Content -Raw -LiteralPath $ProjectPath
[xml]$optionsCheck = Get-Content -Raw -LiteralPath $OptionsPath
if (@($projectCheck.Project.Targets.Target).Count -ne 4 -or
    @($optionsCheck.ProjectOpt.Target).Count -ne 4) {
    throw "Keil project and user-options target counts are not synchronized to four targets"
}
$expectedTargets = @(
    "STM32H743_Audio_SelfTest",
    "STM32H743_Audio_WM8960_Stream",
    "STM32H743_Audio_Generic_DSP",
    "STM32H743_Audio_LVGL_UI"
)
$analysisManifest = @()
$analysisManifest += @($genericDspSources | ForEach-Object {
    "../App/Generic_DSP/" + $_
})
$analysisManifest += "../App/Src/generic_dsp_selftest.c"
$analysisManifest += @($genericDspHeaders | ForEach-Object {
    "../App/Generic_DSP/" + $_
})
$analysisManifest += "../App/Src/generic_dsp_selftest.h"
$lvglManifest = @($lvglSources | ForEach-Object { $_.Path })
$projectTargetNames = @($projectCheck.Project.Targets.Target |
    ForEach-Object TargetName)
$optionTargetNames = @($optionsCheck.ProjectOpt.Target |
    ForEach-Object TargetName)
foreach ($expectedTarget in $expectedTargets) {
    if (($projectTargetNames -notcontains $expectedTarget) -or
        ($optionTargetNames -notcontains $expectedTarget)) {
        throw "Keil project/options target is missing: $expectedTarget"
    }
}
foreach ($targetCheck in @($projectCheck.Project.Targets.Target)) {
    $targetName = [string]$targetCheck.TargetName
    $appC = @($targetCheck.Groups.Group | Where-Object {
        $_.GroupName -eq "Application/Audio"
    } | ForEach-Object { $_.Files.File } | Where-Object {
        $_.FileType -eq "1"
    })
    $permissiveC = @($targetCheck.Groups.Group | Where-Object {
        $_.GroupName -eq "Library/DSP permissive"
    } | ForEach-Object { $_.Files.File } | Where-Object {
        $_.FileType -eq "1"
    })
    if (($appC.Count -ne 3) -or ($permissiveC.Count -ne 49)) {
        throw "Incomplete App/permissive manifest in $targetName"
    }
    $targetDefines = [string]$targetCheck.TargetOption.TargetArmAds.Cads.VariousControls.Define
    $expectedStream = if ($targetName -eq
        "STM32H743_Audio_WM8960_Stream") { "1" } else { "0" }
    $expectedAnalysis = if ($targetName -eq
        "STM32H743_Audio_Generic_DSP") { "1" } else { "0" }
    foreach ($defineContract in @(
        "AUDIO_BOARD_ENABLE_WM8960_STREAM=$expectedStream",
        "AUDIO_BOARD_ENABLE_GENERIC_DSP=$expectedAnalysis"
    )) {
        if ($targetDefines -notmatch
            "(?:^|,)$([regex]::Escape($defineContract))(?:,|$)") {
            throw "Wrong define matrix in $targetName : $defineContract"
        }
    }
    $targetIncludes = @(([string]$targetCheck.TargetOption.TargetArmAds.Cads.VariousControls.IncludePath) -split ";")
    $hasGenericDspInclude = $targetIncludes -contains $genericDspInclude
    if ($hasGenericDspInclude -ne ($expectedAnalysis -eq "1")) {
        throw "Generic_DSP include isolation failed in $targetName"
    }
    $analysisGroups = @($targetCheck.Groups.Group | Where-Object {
        $_.GroupName -eq "Library/Generic DSP"
    })
    if ($analysisGroups.Count -ne 1) {
        throw "Every Keil target must contain one synchronized Generic_DSP group"
    }
    $analysisFiles = @($analysisGroups[0].Files.File)
    $analysisC = @($analysisFiles | Where-Object { $_.FileType -eq "1" })
    $analysisHeaders = @($analysisFiles | Where-Object { $_.FileType -eq "5" })
    $actualManifest = @($analysisFiles | ForEach-Object { [string]$_.FilePath })
    if (($analysisC.Count -ne 14) -or ($analysisHeaders.Count -ne 15) -or
        (($actualManifest -join "|") -ne ($analysisManifest -join "|"))) {
        throw "Generic_DSP target file tree is not the synchronized 14-C/15-header manifest"
    }
    foreach ($analysisFile in $analysisC) {
        $includeNode = $analysisFile.SelectSingleNode(
            "FileOption/CommonProperty/IncludeInBuild")
        if ($null -eq $includeNode -or
            [string]$includeNode.InnerText -ne [string][int]$expectedAnalysis) {
            throw "Generic_DSP IncludeInBuild flag is wrong in $targetName"
        }
    }
    $lvglGroups = @($targetCheck.Groups.Group | Where-Object {
        $_.GroupName -eq "Library/LVGL v9.5"
    })
    if ($lvglGroups.Count -ne 1) {
        throw "Every Keil target must contain one synchronized LVGL v9.5 group"
    }
    $lvglFiles = @($lvglGroups[0].Files.File)
    $lvglC = @($lvglFiles | Where-Object { $_.FileType -eq "1" })
    $actualLvglManifest = @($lvglFiles | ForEach-Object { [string]$_.FilePath })
    if (($lvglC.Count -ne $lvglManifest.Count) -or
        (($actualLvglManifest -join "|") -ne ($lvglManifest -join "|"))) {
        throw "LVGL target file tree is not the synchronized v9.5 C-source manifest"
    }
    $expectedLvgl = if ($targetName -eq "STM32H743_Audio_LVGL_UI") { "1" } else { "0" }
    $lvglDefine = "AUDIO_BOARD_ENABLE_LVGL_UI=$expectedLvgl"
    if ($targetDefines -notmatch
        "(?:^|,)$([regex]::Escape($lvglDefine))(?:,|$)") {
        throw "Wrong LVGL define matrix in $targetName"
    }
    $hasLvglInclude = $true
    foreach ($lvglInclude in $lvglIncludePaths) {
        if ($targetIncludes -notcontains $lvglInclude) {
            $hasLvglInclude = $false
        }
    }
    if ($hasLvglInclude -ne ($expectedLvgl -eq "1") -or
        ($expectedLvgl -eq "0" -and
         @($targetIncludes | Where-Object { $_ -in $lvglIncludePaths }).Count -ne 0)) {
        throw "LVGL include isolation failed in $targetName"
    }
    foreach ($lvglFile in $lvglC) {
        $includeNode = $lvglFile.SelectSingleNode(
            "FileOption/CommonProperty/IncludeInBuild")
        if ($null -eq $includeNode -or
            [string]$includeNode.InnerText -ne [string][int]$expectedLvgl) {
            throw "LVGL IncludeInBuild flag is wrong in $targetName"
        }
    }
    $suppressedFiles = @($lvglC | Where-Object {
        $node = $_.SelectSingleNode(
            "FileOption/FileArmAds/Cads/VariousControls/MiscControls")
        ($null -ne $node) -and -not [string]::IsNullOrWhiteSpace($node.InnerText)
    })
    if ($expectedLvgl -eq "1") {
        $expectedSuppressions = @{
            "lv_draw_sw_blend_to_rgb565.c" = "-Wno-unused-function"
            "lv_draw_sw_blend_to_rgb565_swapped.c" = "-Wno-unused-function"
            "lv_tlsf.c" = "-Wno-unused-parameter"
        }
        if ($suppressedFiles.Count -ne $expectedSuppressions.Count) {
            throw "LVGL target must have exactly three file-local warning suppressions"
        }
        foreach ($suppressedFile in $suppressedFiles) {
            $fileName = [string]$suppressedFile.FileName
            $actualFlags = [string]$suppressedFile.SelectSingleNode(
                "FileOption/FileArmAds/Cads/VariousControls/MiscControls").InnerText
            if ((-not $expectedSuppressions.ContainsKey($fileName)) -or
                ($actualFlags -ne $expectedSuppressions[$fileName])) {
                throw "Unexpected LVGL file-local compiler flags: $fileName $actualFlags"
            }
        }
    }
    elseif ($suppressedFiles.Count -ne 0) {
        throw "Non-LVGL target unexpectedly contains LVGL warning suppressions"
    }
}
if (@($projectCheck.SelectNodes("/Project/Targets/Target/Group")).Count -ne 0) {
    throw "A Keil Group was written directly below Target"
}
if (@($projectCheck.SelectNodes(
    "/Project/Targets/Target/Groups/Group/Files/File[not(FilePath) or normalize-space(FilePath)='']")).Count -ne 0) {
    throw "Keil project contains an empty FilePath"
}
$cmsisComponents = @($projectCheck.Project.RTE.components.component |
    Where-Object { $_.Cclass -eq "CMSIS" })
if ($cmsisComponents.Count -ne 2) {
    throw "Keil RTE must contain exactly CMSIS CORE and DSP components"
}
foreach ($component in $cmsisComponents) {
    $targetInfoNames = @($component.targetInfos.targetInfo |
        ForEach-Object name)
    if (($targetInfoNames.Count -ne 1) -or
        ($targetInfoNames[0] -ne "STM32H743_Audio_Generic_DSP")) {
        throw "CMSIS RTE component is not isolated to the Generic_DSP target"
    }
}
$coreComponent = @($cmsisComponents | Where-Object {
    ($_.Cgroup -eq "CORE") -and ($_.Cversion -eq "6.2.0") -and
    ($_.package.name -eq "CMSIS") -and ($_.package.version -eq "6.3.0")
})
$dspComponent = @($cmsisComponents | Where-Object {
    ($_.Cgroup -eq "DSP") -and ($_.Cvariant -eq "Source") -and
    ($_.Cversion -eq "1.16.2") -and ($_.package.name -eq "CMSIS-DSP") -and
    ($_.package.version -eq "1.16.2")
})
if (($coreComponent.Count -ne 1) -or ($dspComponent.Count -ne 1)) {
    throw "CMSIS RTE component versions do not match the locked Pack contract"
}
$currentOptions = @($optionsCheck.ProjectOpt.Target | Where-Object {
    $_.TargetOption.OPTFL.IsCurrentTarget -eq "1"
})
if (($currentOptions.Count -ne 1) -or
    ($currentOptions[0].TargetName -ne "STM32H743_Audio_SelfTest")) {
    throw "Keil user options must select only the SelfTest target by default"
}

Write-Host "Configured synchronized four-target Keil MDK project for ArmClang 6.24, App sources, Generic DSP analysis and isolated LVGL v9.5 UI."
