param(
    [ValidateSet("SelfTest", "WM8960", "Both")]
    [string]$Target = "Both",
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
                         "STM32H743_Audio_WM8960_Stream")
if ($projectTargetNames.Count -ne 2 -or $optionsTargetNames.Count -ne 2 -or
    @($requiredTargetNames | Where-Object {
        $projectTargetNames -notcontains $_ -or
        $optionsTargetNames -notcontains $_
    }).Count -ne 0) {
    throw "Keil .uvprojx/.uvoptx are not synchronized to the two required targets"
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
    @{ Key = "WM8960"; Name = "STM32H743_Audio_WM8960_Stream" }
)
if ($Target -ne "Both") {
    $targetSpecs = @($targetSpecs | Where-Object { $_.Key -eq $Target })
}

foreach ($spec in $targetSpecs) {
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
                        "-j0", "-o", $mdkLog) -PassThru
    Wait-Process -Id $build.Id -Timeout 120 -ErrorAction SilentlyContinue
    if (Get-Process -Id $build.Id -ErrorAction SilentlyContinue) {
        Stop-Process -Id $build.Id -Force
        throw "Keil $($spec.Name) rebuild exceeded 120 seconds"
    }
    if (-not (Test-Path -LiteralPath $mdkLog)) {
        throw "Keil did not create a build log for $($spec.Name)"
    }
    $log = Get-Content -Raw -LiteralPath $mdkLog
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
    Copy-Item -LiteralPath $mdkLog -Destination $targetLog -Force
    Write-Host $log
    Write-Host "Keil target $($spec.Name): 12.288 MHz MCLK / 3.072 MHz BCLK contract, 49 portable DSP objects and D2 SRAM DMA placement verified."
}
