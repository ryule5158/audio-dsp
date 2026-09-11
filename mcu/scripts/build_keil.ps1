param(
    [ValidateSet("SelfTest", "WM8960", "Generic_DSP", "LVGL", "Both", "All")]
    [string]$Target = "All",
    [switch]$DetailedLog,
    [string]$Uv4Exe = (Join-Path $env:LOCALAPPDATA "Keil_v5\UV4\UV4.exe"),
    [string]$ArmClangExe = (Join-Path $env:LOCALAPPDATA `
        "Keil_v5\ARM\ARMCLANG\Bin\armclang.exe")
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$mcuRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$mdkDir = (Resolve-Path -LiteralPath (Join-Path $mcuRoot `
    "project\STM32H743_Audio\MDK-ARM")).Path
$projectDir = (Resolve-Path -LiteralPath (Join-Path $mdkDir "..")).Path
$projectPath = Join-Path $mdkDir "STM32H743_Audio.uvprojx"
$optionsPath = Join-Path $mdkDir "STM32H743_Audio.uvoptx"
$iocPath = Join-Path $projectDir "STM32H743_Audio.ioc"
$saiPath = Join-Path $projectDir "Core\Src\sai.c"
$mdkLog = Join-Path $mdkDir "keil_build.log"
$evidenceDir = Join-Path $mcuRoot "build"

foreach ($required in @($uv4Exe, $armclangExe, $projectPath, $optionsPath,
                         $iocPath, $saiPath)) {
    if (-not (Test-Path -LiteralPath $required)) {
        throw "Required Keil input is missing: $required"
    }
}
$uvVersion = (Get-Item -LiteralPath $uv4Exe).VersionInfo.ProductVersion
if (-not $uvVersion.StartsWith("5.43")) {
    throw "Expected Keil MDK 5.43, found $uvVersion"
}
$clangVersion = (& $armclangExe --version 2>&1 | Out-String)
if ($clangVersion -notmatch "6\.24") {
    throw "Expected ArmClang 6.24: $clangVersion"
}

$iocText = Get-Content -Raw -LiteralPath $iocPath
foreach ($expected in @(
    "Mcu.CPN=STM32H743IIT6",
    "RCC.SAI1Freq_Value=12288000",
    "SAI1.MClockEnable-SAI_A_MasterWithClock=SAI_MASTERCLOCK_ENABLE",
    "PE3.GPIO_Speed=GPIO_SPEED_FREQ_VERY_HIGH",
    "PE4.GPIO_Speed=GPIO_SPEED_FREQ_VERY_HIGH",
    "PE5.GPIO_Speed=GPIO_SPEED_FREQ_VERY_HIGH",
    "PE6.GPIO_Speed=GPIO_SPEED_FREQ_VERY_HIGH",
    "PG7.GPIO_Speed=GPIO_SPEED_FREQ_VERY_HIGH"
)) {
    if (-not $iocText.Contains($expected)) {
        throw "IOC audio-clock/GPIO contract is missing: $expected"
    }
}
$saiText = Get-Content -Raw -LiteralPath $saiPath
foreach ($expected in @(
    "HAL_SAI_InitProtocol(&hsai_BlockA1, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_24BIT, 2)",
    "HAL_SAI_InitProtocol(&hsai_BlockB1, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_24BIT, 2)"
)) {
    if (-not $saiText.Contains($expected)) {
        throw "Generated SAI frame contract is missing: $expected"
    }
}
if ([regex]::Matches($saiText,
    "GPIO_InitStruct\.Speed = GPIO_SPEED_FREQ_VERY_HIGH;").Count -ne 3) {
    throw "Generated SAI pins are not all configured at very-high GPIO speed"
}
[xml]$projectXml = Get-Content -Raw -LiteralPath $projectPath
[xml]$optionsXml = Get-Content -Raw -LiteralPath $optionsPath
$projectTargetNames = @($projectXml.Project.Targets.Target |
    ForEach-Object TargetName)
$optionsTargetNames = @($optionsXml.ProjectOpt.Target |
    ForEach-Object TargetName)
$requiredTargetNames = @("STM32H743_Audio_SelfTest",
                         "STM32H743_Audio_WM8960_Stream",
                         "STM32H743_Audio_Generic_DSP",
                         "STM32H743_Audio_LVGL_UI")
if ($projectTargetNames.Count -ne 4 -or $optionsTargetNames.Count -ne 4 -or
    @($requiredTargetNames | Where-Object {
        $projectTargetNames -notcontains $_ -or
        $optionsTargetNames -notcontains $_
    }).Count -ne 0) {
    throw "Keil .uvprojx/.uvoptx are not synchronized to the four required targets"
}
$cmsisPack = Join-Path $env:LOCALAPPDATA "Arm\Packs\ARM\CMSIS\6.3.0"
$dspPack = Join-Path $env:LOCALAPPDATA "Arm\Packs\ARM\CMSIS-DSP\1.16.2"
foreach ($packInput in @(
    (Join-Path $cmsisPack "ARM.CMSIS.pdsc"),
    (Join-Path $dspPack "ARM.CMSIS-DSP.pdsc")
)) {
    if (-not (Test-Path -LiteralPath $packInput)) {
        throw "Required CMSIS Pack input is missing: $packInput"
    }
}
$cmsisComponents = @($projectXml.Project.RTE.components.component |
    Where-Object { $_.Cclass -eq "CMSIS" })
if ($cmsisComponents.Count -ne 2 -or
    @($cmsisComponents | Where-Object {
        $_.targetInfos.targetInfo.name -ne "STM32H743_Audio_Generic_DSP"
    }).Count -ne 0) {
    throw "CMSIS RTE components are not isolated to the Generic_DSP target"
}
foreach ($targetNode in @($projectXml.Project.Targets.Target)) {
    $targetName = [string]$targetNode.TargetName
    $expectedStream = if ($targetName -eq
        "STM32H743_Audio_WM8960_Stream") { "1" } else { "0" }
    $expectedAnalysis = if ($targetName -eq
        "STM32H743_Audio_Generic_DSP") { "1" } else { "0" }
    $defines = [string]$targetNode.TargetOption.TargetArmAds.Cads.VariousControls.Define
    foreach ($defineContract in @(
        "AUDIO_BOARD_ENABLE_WM8960_STREAM=$expectedStream",
        "AUDIO_BOARD_ENABLE_GENERIC_DSP=$expectedAnalysis"
    )) {
        if ($defines -notmatch
            "(?:^|,)$([regex]::Escape($defineContract))(?:,|$)") {
            throw "Wrong define matrix in $targetName : $defineContract"
        }
    }
    $analysisGroups = @($targetNode.Groups.Group | Where-Object {
        $_.GroupName -eq "Library/Generic DSP"
    })
    if ($analysisGroups.Count -ne 1) {
        throw "Every Keil target must contain one synchronized Generic_DSP group"
    }
    $analysisFiles = @($analysisGroups[0].Files.File)
    $analysisC = @($analysisFiles | Where-Object { $_.FileType -eq "1" })
    $analysisHeaders = @($analysisFiles | Where-Object { $_.FileType -eq "5" })
    if (($analysisC.Count -ne 14) -or ($analysisHeaders.Count -ne 15)) {
        throw "Generic_DSP target source manifest is incomplete"
    }
    foreach ($analysisFile in $analysisC) {
        $includeNode = $analysisFile.SelectSingleNode(
            "FileOption/CommonProperty/IncludeInBuild")
        if ($null -eq $includeNode -or
            [string]$includeNode.InnerText -ne [string][int]$expectedAnalysis) {
            throw "Generic_DSP IncludeInBuild flag is wrong in $targetName"
        }
    }
    $includes = @(([string]$targetNode.TargetOption.TargetArmAds.Cads.VariousControls.IncludePath) -split ";")
    if (($includes -contains "../App/Generic_DSP") -ne ($expectedAnalysis -eq "1")) {
        throw "Wrong Generic_DSP include isolation in $targetName"
    }
    $lvglGroups = @($targetNode.Groups.Group | Where-Object {
        $_.GroupName -eq "Library/LVGL v9.5"
    })
    if ($lvglGroups.Count -ne 1) {
        throw "Every target must contain one synchronized LVGL v9.5 group"
    }
    $lvglC = @($lvglGroups[0].Files.File | Where-Object { $_.FileType -eq "1" })
    if ($lvglC.Count -lt 3) {
        throw "LVGL v9.5 source manifest is unexpectedly small in $targetName"
    }
    $expectedLvgl = if ($targetName -eq "STM32H743_Audio_LVGL_UI") { "1" } else { "0" }
    $lvglDefine = "AUDIO_BOARD_ENABLE_LVGL_UI=$expectedLvgl"
    if ($defines -notmatch
        "(?:^|,)$([regex]::Escape($lvglDefine))(?:,|$)") {
        throw "Wrong LVGL define matrix in $targetName"
    }
    foreach ($lvglInclude in @("../App/LVGL", "../App/LVGL/src", "../App/LVGL_UI")) {
        if (($includes -contains $lvglInclude) -ne ($expectedLvgl -eq "1")) {
            throw "Wrong LVGL include isolation in $targetName : $lvglInclude"
        }
    }
    foreach ($lvglOnlyDefine in @("HAL_SPI_MODULE_ENABLED", "LV_CONF_INCLUDE_SIMPLE")) {
        $hasDefine = $defines -match "(?:^|,)$lvglOnlyDefine(?:,|$)"
        if ($hasDefine -ne ($expectedLvgl -eq "1")) {
            throw "Wrong LVGL-only define in $targetName : $lvglOnlyDefine"
        }
    }
    foreach ($lvglFile in $lvglC) {
        $includeNode = $lvglFile.SelectSingleNode(
            "FileOption/CommonProperty/IncludeInBuild")
        if ($null -eq $includeNode -or
            [string]$includeNode.InnerText -ne [string][int]$expectedLvgl) {
            throw "LVGL IncludeInBuild flag is wrong in $targetName"
        }
    }
}
$debugDir = Join-Path $mdkDir "DebugConfig"
foreach ($requiredTargetName in $requiredTargetNames) {
    $debugPath = Join-Path $debugDir `
        ($requiredTargetName + "_STM32H743IITx_1.1.1.dbgconf")
    if (-not (Test-Path -LiteralPath $debugPath)) {
        throw "Keil debugger configuration is missing: $debugPath"
    }
}

[void](New-Item -ItemType Directory -Path $evidenceDir -Force)

$permissiveSources = @(
    "aud_adenv", "aud_adsr", "aud_analogbassdrum", "aud_analogsnaredrum",
    "aud_autowah", "aud_chorus", "aud_clockednoise", "aud_crossfade",
    "aud_dcblock", "aud_decimator", "aud_drip", "aud_dust", "aud_flanger",
    "aud_fm2", "aud_formantosc", "aud_fractal_noise", "aud_grainlet",
    "aud_granularplayer", "aud_harmonic_osc", "aud_hihat", "aud_karplusstring",
    "aud_limiter", "aud_looper", "aud_maytrig", "aud_metro", "aud_modalvoice",
    "aud_osc", "aud_oscillatorbank", "aud_overdrive", "aud_particle",
    "aud_phaser", "aud_phasor", "aud_pitchshifter", "aud_resonator",
    "aud_samplehold", "aud_sampleratereducer", "aud_smooth_random", "aud_soap",
    "aud_stringvoice", "aud_svf", "aud_synthbassdrum", "aud_synthsnaredrum",
    "aud_tremolo", "aud_variablesawosc", "aud_variableshapeosc", "aud_vosim",
    "aud_wavefolder", "aud_whitenoise", "aud_zoscillator"
)
if ($permissiveSources.Count -ne 49) { throw "Expected 49 permissive DSP objects" }

$targetSpecs = @(
    @{ Key = "SelfTest"; Name = "STM32H743_Audio_SelfTest" },
    @{ Key = "WM8960"; Name = "STM32H743_Audio_WM8960_Stream" },
    @{ Key = "Generic_DSP"; Name = "STM32H743_Audio_Generic_DSP" },
    @{ Key = "LVGL"; Name = "STM32H743_Audio_LVGL_UI" }
)
if ($Target -eq "Both") {
    $targetSpecs = @($targetSpecs | Where-Object {
        $_.Key -in @("SelfTest", "WM8960")
    })
} elseif ($Target -ne "All") {
    $targetSpecs = @($targetSpecs | Where-Object { $_.Key -eq $Target })
}
$genericDspObjects = @(
    "Adaptive", "Correlate", "Demod", "DSP_ProMax", "FFT", "Filter",
    "FilterEx", "Fit", "IQ", "Measure", "ModelFit", "periodic_analyzer",
    "SoftPll", "generic_dsp_selftest"
)
$lvglCPaths = @($projectXml.Project.Targets.Target[0].Groups.Group |
    Where-Object { $_.GroupName -eq "Library/LVGL v9.5" } |
    ForEach-Object { $_.Files.File } |
    Where-Object { $_.FileType -eq "1" } |
    ForEach-Object { [string]$_.FilePath })
if ($lvglCPaths.Count -lt 3) {
    throw "LVGL v9.5 source manifest is missing from the project"
}

function Get-DependencySourceCounts {
    param([string]$OutputDirectory)

    $sourceCounts = @{}
    foreach ($dependency in @(Get-ChildItem -LiteralPath $OutputDirectory `
            -File -Filter "*.d" -ErrorAction SilentlyContinue)) {
        $dependencyText = (Get-Content -Raw -LiteralPath $dependency.FullName) `
            -replace "\\\r?\n", " "
        $objectMatch = [regex]::Match($dependencyText, '^([^\r\n]+?\.o):')
        if (-not $objectMatch.Success) {
            throw "Dependency file has no object target: $($dependency.FullName)"
        }
        $objectPath = [IO.Path]::GetFullPath(
            (Join-Path $mdkDir $objectMatch.Groups[1].Value.Trim()))
        $outputPrefix = [IO.Path]::GetFullPath($OutputDirectory) +
            [IO.Path]::DirectorySeparatorChar
        if (-not $objectPath.StartsWith($outputPrefix,
                [StringComparison]::OrdinalIgnoreCase) -or
            -not (Test-Path -LiteralPath $objectPath -PathType Leaf)) {
            throw "Dependency target is missing or outside the output directory: $objectPath"
        }
        foreach ($match in [regex]::Matches(
                $dependencyText, '(?i)(?:^|\s)([^\s:]+\.c)(?=\s|$)')) {
            $sourceToken = $match.Groups[1].Value
            $sourceFullPath = ([IO.Path]::GetFullPath(
                (Join-Path $mdkDir $sourceToken)) -replace '\\', '/').ToLowerInvariant()
            if ($sourceCounts.ContainsKey($sourceFullPath)) {
                $sourceCounts[$sourceFullPath]++
            }
            else {
                $sourceCounts[$sourceFullPath] = 1
            }
        }
    }
    return $sourceCounts
}

foreach ($spec in $targetSpecs) {
    $runningUv4 = @(Get-Process -Name UV4 -ErrorAction SilentlyContinue)
    if ($runningUv4.Count -ne 0) {
        throw "Close the existing uVision instance before a clean rebuild; no process was stopped"
    }
    $outputDir = Join-Path $mdkDir $spec.Name
    $targetLog = Join-Path $evidenceDir ("keil_{0}.log" -f $spec.Key.ToLowerInvariant())
    if (Test-Path -LiteralPath $outputDir) {
        $resolvedOutput = (Resolve-Path -LiteralPath $outputDir).Path
        $mdkPrefix = $mdkDir + [IO.Path]::DirectorySeparatorChar
        if (-not $resolvedOutput.StartsWith($mdkPrefix,
            [System.StringComparison]::OrdinalIgnoreCase)) {
            throw "Refusing unexpected build-output path: $resolvedOutput"
        }
        Remove-Item -LiteralPath $resolvedOutput -Recurse -Force
    }
    Remove-Item -LiteralPath $mdkLog -Force -ErrorAction SilentlyContinue
    $build = Start-Process -FilePath $uv4Exe -WorkingDirectory $mdkDir `
        -ArgumentList @("-r", $projectPath, "-t", $spec.Name,
                        "-j0", "-o", $mdkLog) -WindowStyle Hidden -PassThru
    Wait-Process -Id $build.Id -Timeout 300 -ErrorAction SilentlyContinue
    if (Get-Process -Id $build.Id -ErrorAction SilentlyContinue) {
        Stop-Process -Id $build.Id -Force
        throw "Keil $($spec.Name) rebuild exceeded 300 seconds"
    }
    if (-not (Test-Path -LiteralPath $mdkLog)) {
        throw "Keil did not create a build log for $($spec.Name)"
    }
    $log = Get-Content -Raw -LiteralPath $mdkLog
    Copy-Item -LiteralPath $mdkLog -Destination $targetLog -Force
    if ($log -notmatch "0 Error\(s\), 0 Warning\(s\)\.") {
        throw "Keil build did not meet the 0-error/0-warning gate for $($spec.Name)`n$log"
    }
    $mapPath = Join-Path $outputDir ($spec.Name + ".map")
    if (-not (Test-Path -LiteralPath $mapPath)) {
        throw "Keil map file is missing for $($spec.Name)"
    }
    $map = Get-Content -Raw -LiteralPath $mapPath
    foreach ($expected in @(
        "Execution Region RW_AUDIO_DMA",
        "Size: 0x00000800, Max: 0x00008000",
        "s_rx                                     0x30000000",
        "s_tx                                     0x30000400"
    )) {
        if (-not $map.Contains($expected)) {
            throw "DMA placement evidence is missing from map: $expected"
        }
    }
    $missing = @($permissiveSources | Where-Object {
        -not (Test-Path -LiteralPath (Join-Path $outputDir ($_ + ".o")))
    })
    if ($missing.Count -ne 0) {
        throw "The Keil target did not compile all 49 permissive DSP objects: $($missing -join ', ')"
    }
    $missingGenericDsp = @($genericDspObjects | Where-Object {
        -not (Test-Path -LiteralPath (Join-Path $outputDir ($_ + ".o")))
    })
    if (($spec.Key -eq "Generic_DSP") -and ($missingGenericDsp.Count -ne 0)) {
        throw "The Generic_DSP target did not compile all 14 analysis objects: $($missingGenericDsp -join ', ')"
    }
    if (($spec.Key -ne "Generic_DSP") -and ($missingGenericDsp.Count -ne $genericDspObjects.Count)) {
        throw "A non-analysis target unexpectedly compiled Generic_DSP objects"
    }
    $dependencySources = Get-DependencySourceCounts -OutputDirectory $outputDir
    $lvglDependencyMatches = 0
    foreach ($lvglPath in $lvglCPaths) {
        $lvglFullPath = ([IO.Path]::GetFullPath(
            (Join-Path $mdkDir $lvglPath)) -replace '\\', '/').ToLowerInvariant()
        $actualCount = if ($dependencySources.ContainsKey($lvglFullPath)) {
            [int]$dependencySources[$lvglFullPath]
        }
        else {
            0
        }
        $expectedCount = if ($spec.Key -eq "LVGL") { 1 } else { 0 }
        if ($actualCount -ne $expectedCount) {
            throw "LVGL dependency isolation failed for $($spec.Name): $lvglPath expected $expectedCount, found $actualCount"
        }
        $lvglDependencyMatches += $actualCount
    }
    $expectedLvglObjects = if ($spec.Key -eq "LVGL") { $lvglCPaths.Count } else { 0 }
    if ($lvglDependencyMatches -ne $expectedLvglObjects) {
        throw "LVGL dependency isolation count failed for $($spec.Name): expected $expectedLvglObjects, found $lvglDependencyMatches"
    }
    if ($DetailedLog) {
        Write-Host $log
    } else {
        ($log -split '\r?\n' | Where-Object {
            $_ -match 'Using Compiler|Rebuild target|Program Size|Error\(s\)|Build Time'
        }) | ForEach-Object { Write-Host $_ }
        Write-Host "Full build log: $targetLog"
    }
    $analysisEvidence = if ($spec.Key -eq "Generic_DSP") {
        ", 14 Generic DSP objects"
    } else {
        ""
    }
    Write-Host "Keil target $($spec.Name): 49 portable DSP objects$analysisEvidence; LVGL source/object matches=$lvglDependencyMatches; audio clock and D2 SRAM DMA map contracts verified (not hardware measurements)."
}
