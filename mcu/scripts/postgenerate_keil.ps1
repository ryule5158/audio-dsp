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
    "Group[GroupName='Application/Audio' or GroupName='Library/DSP permissive']"))) {
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

# CubeMX emits one target.  Keep two explicit, reproducible variants so a
# developer can build the safe bench self-test or the real WM8960 stream from
# the same generated project without editing a header by hand.
$targets = $document.SelectSingleNode("/Project/Targets")
$baseTarget = $targets.SelectSingleNode("Target")
if ($null -eq $baseTarget) {
    throw "CubeMX generated no Keil target"
}
$targetCopies = @()
foreach ($spec in @(
    @("STM32H743_Audio_SelfTest", "STM32H743_Audio_SelfTest\", "AUDIO_BOARD_ENABLE_WM8960_STREAM=0"),
    @("STM32H743_Audio_WM8960_Stream", "STM32H743_Audio_WM8960_Stream\", "AUDIO_BOARD_ENABLE_WM8960_STREAM=1")
)) {
    $copy = $baseTarget.CloneNode($true)
    $copy.SelectSingleNode("TargetName").InnerText = $spec[0]
    $copy.SelectSingleNode("TargetOption/TargetCommonOption/OutputDirectory").InnerText = $spec[1]
    $copy.SelectSingleNode("TargetOption/TargetCommonOption/OutputName").InnerText = $spec[0]
    $copy.SelectSingleNode("TargetOption/TargetCommonOption/ListingPath").InnerText = "./" + $spec[0] + "/"
    $define = $copy.SelectSingleNode("TargetOption/TargetArmAds/Cads/VariousControls/Define")
    $baseDefines = @($define.InnerText -split "," |
        Where-Object { $_ -notmatch "^AUDIO_BOARD_ENABLE_WM8960_STREAM=" })
    $define.InnerText = (($baseDefines + $spec[2]) -join ",")
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
    @("STM32H743_Audio_WM8960_Stream", "0")
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
# Materialize debugger configuration for both final targets and remove the
# obsolete base-target files so opening the project never exposes a dead name.
$debugDir = Join-Path $mdkDir "DebugConfig"
$baseDebug = Join-Path $debugDir `
    "STM32H743_Audio_STM32H743IITx_1.1.1.dbgconf"
if (Test-Path -LiteralPath $baseDebug) {
    foreach ($targetName in @("STM32H743_Audio_SelfTest",
                               "STM32H743_Audio_WM8960_Stream")) {
        Copy-Item -LiteralPath $baseDebug -Destination (Join-Path $debugDir `
            ($targetName + "_STM32H743IITx_1.1.1.dbgconf")) -Force
    }
    Remove-Item -LiteralPath $baseDebug -Force
}
$obsoleteRte = Join-Path $mdkDir "RTE\_STM32H743_Audio"
if (Test-Path -LiteralPath $obsoleteRte) {
    Remove-Item -LiteralPath $obsoleteRte -Recurse -Force
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
if (@($projectCheck.Project.Targets.Target).Count -ne 2 -or
    @($optionsCheck.ProjectOpt.Target).Count -ne 2) {
    throw "Keil project and user-options target counts are not synchronized"
}

Write-Host "Configured synchronized two-target Keil MDK project for ArmClang 6.24 and App sources."
