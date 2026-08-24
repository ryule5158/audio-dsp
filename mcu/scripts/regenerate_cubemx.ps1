param(
    [string]$CubeMxExe = (Join-Path $env:ProgramFiles `
        "STMicroelectronics\STM32Cube\STM32CubeMX\STM32CubeMX.exe"),
    [string]$CubePackage = (Join-Path $env:USERPROFILE `
        "STM32Cube\Repository\STM32Cube_FW_H7_V1.13.0"),
    [string]$CubeLog = (Join-Path $env:USERPROFILE `
        ".stm32cubemx\STM32CubeMX.log")
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectDir = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot `
    "..\project\STM32H743_Audio")).Path
$iocPath = Join-Path $projectDir "STM32H743_Audio.ioc"
$mainPath = Join-Path $projectDir "Core\Src\main.c"
$saiPath = Join-Path $projectDir "Core\Src\sai.c"
$uvprojxPath = Join-Path $projectDir "MDK-ARM\STM32H743_Audio.uvprojx"
$uvoptxPath = Join-Path $projectDir "MDK-ARM\STM32H743_Audio.uvoptx"
$mcuRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..")).Path
$buildDir = Join-Path $mcuRoot "build"
foreach ($required in @($cubeMxExe, $cubePackage, $iocPath)) {
    if (-not (Test-Path -LiteralPath $required)) {
        throw "Required CubeMX input is missing: $required"
    }
}
$cubeVersion = (Get-Item -LiteralPath $cubeMxExe).VersionInfo.ProductVersion
if ($cubeVersion -notmatch "^>?6\.17\.0(?:-|$)") {
    throw "Expected STM32CubeMX 6.17.0, found $cubeVersion"
}
$ioc = Get-Content -Raw -LiteralPath $iocPath
foreach ($expected in @(
    "MxCube.Version=6.17.0",
    "ProjectManager.FirmwarePackage=STM32Cube FW_H7 V1.13.0",
    "Mcu.CPN=STM32H743IIT6"
)) {
    if (-not $ioc.Contains($expected)) {
        throw "IOC version/device contract is missing: $expected"
    }
}

$cubeDir = Split-Path -Parent $cubeMxExe
function Get-CubeJavaProcess {
    return @(Get-Process javaw -ErrorAction SilentlyContinue |
        Where-Object {
            try {
                $_.Path.StartsWith($cubeDir,
                    [System.StringComparison]::OrdinalIgnoreCase)
            } catch {
                $false
            }
        })
}

$existing = @(Get-CubeJavaProcess)
if ($existing.Count -ne 0) {
    throw "Close the existing STM32CubeMX process before CLI regeneration: $($existing.Id -join ', ')"
}

[void](New-Item -ItemType Directory -Path $buildDir -Force)
$cliPath = Join-Path $buildDir "cubemx_commands.txt"
@(
    "config load $iocPath",
    "project generate",
    "exit"
) | Set-Content -LiteralPath $cliPath -Encoding ASCII

$generatedFiles = @($mainPath, $saiPath, $uvprojxPath)
$beforeWriteUtc = @{}
foreach ($path in $generatedFiles) {
    if (Test-Path -LiteralPath $path) {
        $beforeWriteUtc[$path] = (Get-Item -LiteralPath $path).LastWriteTimeUtc
    }
}
$beforeLogWriteUtc = $null
try {
    if (Test-Path -LiteralPath $cubeLog) {
        $beforeLogWriteUtc = (Get-Item -LiteralPath $cubeLog).LastWriteTimeUtc
    }
} catch {
    $beforeLogWriteUtc = $null
}

$started = Get-Date
$startedUtc = $started.ToUniversalTime()

function Get-GenerationEvidence {
    $allFilesFresh = $true
    foreach ($path in $generatedFiles) {
        if (-not (Test-Path -LiteralPath $path)) {
            $allFilesFresh = $false
            break
        }
        $writeUtc = (Get-Item -LiteralPath $path).LastWriteTimeUtc
        if (($beforeWriteUtc.ContainsKey($path) -and
             $writeUtc -le $beforeWriteUtc[$path]) -or
            $writeUtc -lt $startedUtc.AddSeconds(-2)) {
            $allFilesFresh = $false
            break
        }
    }

    $freshLog = $false
    $freshLogText = ""
    try {
        if (Test-Path -LiteralPath $cubeLog) {
            $logInfo = Get-Item -LiteralPath $cubeLog
            $freshLogText = Get-Content -Raw -LiteralPath $cubeLog
            $freshLog = ($logInfo.LastWriteTimeUtc -ge
                         $startedUtc.AddSeconds(-2))
            if ($null -ne $beforeLogWriteUtc) {
                $freshLog = $freshLog -and
                            ($logInfo.LastWriteTimeUtc -gt $beforeLogWriteUtc)
            }
        }
    } catch {
        $freshLog = $false
        $freshLogText = ""
    }

    $freshCompletionMarker = $freshLog -and
        ($freshLogText -match "Time for Generating toolchain IDE Files")
    return [pscustomobject]@{
        FilesFresh = $allFilesFresh
        LogFresh = $freshLog
        LogText = $freshLogText
        Complete = ($allFilesFresh -or $freshCompletionMarker)
    }
}

$wrapper = Start-Process -FilePath $cubeMxExe `
    -ArgumentList @("-q", $cliPath) -PassThru
$deadline = (Get-Date).AddSeconds(20)
$cubeJava = $null
do {
    $found = @(Get-CubeJavaProcess)
    if ($found.Count -gt 0) {
        $cubeJava = $found | Sort-Object StartTime -Descending |
            Select-Object -First 1
        break
    }
    Start-Sleep -Milliseconds 250
} while ((Get-Date) -lt $deadline)

if ($null -ne $cubeJava) {
    Wait-Process -Id $cubeJava.Id -Timeout 120 -ErrorAction SilentlyContinue
    if (Get-Process -Id $cubeJava.Id -ErrorAction SilentlyContinue) {
        # CubeMX 6.17 can leave the quiet-mode Java launcher resident after it
        # has flushed the project.  Kill only the process started by this run,
        # and accept it only with fresh log or fresh generated-file evidence.
        $timeoutEvidence = Get-GenerationEvidence
        Stop-Process -Id $cubeJava.Id -Force
        if (-not $timeoutEvidence.Complete) {
            throw "CubeMX exceeded 120 seconds without fresh generation evidence; task-owned PID was stopped"
        }
    }
} else {
    Wait-Process -Id $wrapper.Id -Timeout 120 -ErrorAction SilentlyContinue
    if (Get-Process -Id $wrapper.Id -ErrorAction SilentlyContinue) {
        Stop-Process -Id $wrapper.Id -Force
        throw "CubeMX launcher did not spawn Java or finish within 120 seconds"
    }
}

$wrapperStoppedAfterEvidence = $false
if (-not $wrapper.HasExited) {
    Wait-Process -Id $wrapper.Id -Timeout 10 -ErrorAction SilentlyContinue
}
if (Get-Process -Id $wrapper.Id -ErrorAction SilentlyContinue) {
    $wrapperEvidence = Get-GenerationEvidence
    Stop-Process -Id $wrapper.Id -Force
    $wrapperStoppedAfterEvidence = $true
    if (-not $wrapperEvidence.Complete) {
        throw "CubeMX wrapper remained resident without fresh generation evidence"
    }
}

if (-not $wrapperStoppedAfterEvidence -and $wrapper.HasExited -and
    $wrapper.ExitCode -ne 0) {
    throw "CubeMX CLI wrapper exited with code $($wrapper.ExitCode)"
}
$evidence = Get-GenerationEvidence
if (-not $evidence.Complete) {
    throw "CubeMX left no fresh completion marker and did not refresh all key generated files; refusing stale outputs"
}
$blockingPatterns = @(
    "IP\s+.*not ready",
    "invalid value",
    "parameter conflict"
)
if ($evidence.LogFresh) {
    foreach ($pattern in $blockingPatterns) {
        if ($evidence.LogText -match $pattern) {
            throw "CubeMX blocker found in log: $pattern"
        }
    }
    Copy-Item -LiteralPath $cubeLog `
        -Destination (Join-Path $buildDir "cubemx.log") -Force
}

function Assert-Text {
    param(
        [string]$Path,
        [string[]]$Expected
    )

    $text = Get-Content -Raw -LiteralPath $Path
    foreach ($item in $Expected) {
        if (-not $text.Contains($item)) {
            throw "Generated-file contract mismatch in $Path : $item"
        }
    }
}

Assert-Text $iocPath @(
    "Dma.SAI1_A.0.Instance=DMA2_Stream0",
    "Dma.SAI1_A.0.Direction=DMA_MEMORY_TO_PERIPH",
    "Dma.SAI1_A.0.Mode=DMA_CIRCULAR",
    "Dma.SAI1_B.1.Instance=DMA2_Stream1",
    "Dma.SAI1_B.1.Direction=DMA_PERIPH_TO_MEMORY",
    "Dma.SAI1_B.1.Mode=DMA_CIRCULAR",
    "RCC.DIVM3=40",
    "RCC.DIVN3=192",
    "RCC.DIVP3=25",
    "RCC.SAI1Freq_Value=12288000",
    "SAI1.AudioFrequency-SAI_A_MasterWithClock=SAI_AUDIO_FREQUENCY_48K",
    "SAI1.BasicDataSize-SAI_A_MasterWithClock=SAI_PROTOCOL_DATASIZE_24BIT",
    "SAI1.BasicDataSize-SAI_B_SyncSlave=SAI_PROTOCOL_DATASIZE_24BIT",
    "SAI1.MClockEnable-SAI_A_MasterWithClock=SAI_MASTERCLOCK_ENABLE",
    "PE3.GPIO_Speed=GPIO_SPEED_FREQ_VERY_HIGH",
    "PE4.GPIO_Speed=GPIO_SPEED_FREQ_VERY_HIGH",
    "PE5.GPIO_Speed=GPIO_SPEED_FREQ_VERY_HIGH",
    "PE6.GPIO_Speed=GPIO_SPEED_FREQ_VERY_HIGH",
    "PG7.GPIO_Speed=GPIO_SPEED_FREQ_VERY_HIGH"
)
Assert-Text $saiPath @(
    "hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_TX;",
    "hsai_BlockB1.Init.AudioMode = SAI_MODESLAVE_RX;",
    "hsai_BlockB1.Init.Synchro = SAI_SYNCHRONOUS;",
    "hdma_sai1_a.Instance = DMA2_Stream0;",
    "hdma_sai1_a.Init.Request = DMA_REQUEST_SAI1_A;",
    "hdma_sai1_b.Instance = DMA2_Stream1;",
    "hdma_sai1_b.Init.Request = DMA_REQUEST_SAI1_B;",
    "GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;"
)
$saiText = Get-Content -Raw -LiteralPath $saiPath
if ([regex]::Matches($saiText,
    "GPIO_InitStruct\.Speed = GPIO_SPEED_FREQ_VERY_HIGH;").Count -ne 3) {
    throw "Generated SAI GPIO contract requires three very-high-speed GPIO groups"
}
Assert-Text $mainPath @(
    "PeriphClkInitStruct.PLL3.PLL3M = 40;",
    "PeriphClkInitStruct.PLL3.PLL3N = 192;",
    "PeriphClkInitStruct.PLL3.PLL3P = 25;",
    "PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL3;",
    "MPU_InitStruct.BaseAddress = 0x30000000;",
    "#include `"audio_app.h`"",
    "AudioApp_Init();"
)

& (Join-Path $PSScriptRoot "postgenerate_keil.ps1")

[xml]$keil = Get-Content -Raw -LiteralPath $uvprojxPath
$targetNames = @($keil.Project.Targets.Target | ForEach-Object TargetName)
if ($targetNames.Count -ne 2) {
    throw "Keil project must contain exactly two generated targets"
}
foreach ($requiredTarget in @(
    "STM32H743_Audio_SelfTest",
    "STM32H743_Audio_WM8960_Stream"
)) {
    if ($targetNames -notcontains $requiredTarget) {
        throw "Keil target missing after CubeMX regeneration: $requiredTarget"
    }
}
foreach ($target in @($keil.Project.Targets.Target)) {
    $dspFiles = @($target.Groups.Group |
        Where-Object { $_.GroupName -eq "Library/DSP permissive" } |
        ForEach-Object { $_.Files.File } |
        Where-Object { $_.FileType -eq "1" })
    $appFiles = @($target.Groups.Group |
        Where-Object { $_.GroupName -eq "Application/Audio" } |
        ForEach-Object { $_.Files.File })
    if ($dspFiles.Count -ne 49 -or $appFiles.Count -ne 3) {
        throw "Keil target $($target.TargetName) has an incomplete App/DSP source manifest"
    }
    $define = $target.TargetOption.TargetArmAds.Cads.VariousControls.Define
    $expectedStream = if ($target.TargetName -like "*WM8960*") { "1" } else { "0" }
    if ($define -notmatch
        "(?:^|,)AUDIO_BOARD_ENABLE_WM8960_STREAM=$expectedStream(?:,|$)") {
        throw "Keil target $($target.TargetName) has the wrong stream define"
    }
}
[xml]$keilOptions = Get-Content -Raw -LiteralPath $uvoptxPath
$optionTargets = @($keilOptions.ProjectOpt.Target)
if ($optionTargets.Count -ne 2 -or
    @($optionTargets | Where-Object {
        $_.TargetName -in @("STM32H743_Audio_SelfTest",
                            "STM32H743_Audio_WM8960_Stream")
    }).Count -ne 2) {
    throw "Keil user options are not synchronized to the two project targets"
}
Write-Host "CubeMX regeneration and SAI/PLL3/DMA contract checks passed."
