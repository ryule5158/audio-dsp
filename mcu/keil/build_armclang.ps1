param(
    [string]$ArmClang = "C:\Users\LENOVO\AppData\Local\Keil_v5\ARM\ARMCLANG\bin\armclang.exe",
    [string]$ArmAr = "C:\Users\LENOVO\AppData\Local\Keil_v5\ARM\ARMCLANG\bin\armar.exe",
    [string]$CubeH7Root = "C:\Users\LENOVO\STM32Cube\Repository\STM32Cube_FW_H7_V1.13.0"
)

$ErrorActionPreference = "Stop"
$RepoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$OutputDir = Join-Path $RepoRoot "build\mcu-armclang"

if (-not (Test-Path -LiteralPath $ArmClang)) {
    throw "ArmClang not found: $ArmClang"
}
if (-not (Test-Path -LiteralPath $ArmAr)) {
    throw "ArmAr not found: $ArmAr"
}
if (-not (Test-Path -LiteralPath $CubeH7Root)) {
    throw "STM32Cube H7 firmware not found: $CubeH7Root"
}

New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$Common = @(
    "--target=arm-arm-none-eabi",
    "-mcpu=cortex-m7",
    "-mfpu=fpv5-d16",
    "-mfloat-abi=hard",
    "-std=c11",
    "-O2",
    "-ffunction-sections",
    "-fdata-sections",
    "-Wall",
    "-Wextra",
    "-Werror",
    "-I$RepoRoot",
    "-I$(Join-Path $RepoRoot 'mcu\include')"
)

$Sources = @(
    (Join-Path $RepoRoot "mcu\src\aud_mcu_audio.c"),
    (Join-Path $RepoRoot "mcu\src\aud_mcu_fx_chain.c")
)
$Sources += Get-ChildItem -LiteralPath $RepoRoot -Filter "aud_*.c" -File |
    Sort-Object Name |
    Select-Object -ExpandProperty FullName

$Objects = @()
foreach ($Source in $Sources) {
    $Object = Join-Path $OutputDir (([System.IO.Path]::GetFileNameWithoutExtension($Source)) + ".o")
    & $ArmClang @Common -c $Source -o $Object
    if ($LASTEXITCODE -ne 0) {
        throw "ArmClang failed for $Source"
    }
    $Objects += $Object
}

$Library = Join-Path $OutputDir "audio_dsp_h743.lib"
& $ArmAr --create $Library $Objects
if ($LASTEXITCODE -ne 0) {
    throw "ArmAr failed"
}

& $ArmAr -t $Library
Write-Output "MCU_ARMCLANG_BUILD_OK=$Library"

$HalConfig = Join-Path $CubeH7Root "Projects\STM32H743I-EVAL\Examples\SAI\SAI_AudioPlayback\Inc"
$HalIncludes = @(
    "-I$(Join-Path $RepoRoot 'mcu\ports\stm32h743_hal')",
    "-I$HalConfig",
    "-I$(Join-Path $CubeH7Root 'Drivers\STM32H7xx_HAL_Driver\Inc')",
    "-I$(Join-Path $CubeH7Root 'Drivers\STM32H7xx_HAL_Driver\Inc\Legacy')",
    "-I$(Join-Path $CubeH7Root 'Drivers\CMSIS\Device\ST\STM32H7xx\Include')",
    "-I$(Join-Path $CubeH7Root 'Drivers\CMSIS\Include')",
    "-DSTM32H743xx",
    "-DUSE_HAL_DRIVER"
)

$HalSources = @(
    (Join-Path $RepoRoot "mcu\ports\stm32h743_hal\aud_mcu_sai_dma.c"),
    (Join-Path $RepoRoot "mcu\examples\stm32h743_sai_callbacks.c")
)
foreach ($Source in $HalSources) {
    $Object = Join-Path $OutputDir (([System.IO.Path]::GetFileNameWithoutExtension($Source)) + ".o")
    & $ArmClang @Common @HalIncludes -c $Source -o $Object
    if ($LASTEXITCODE -ne 0) {
        throw "ArmClang HAL-port compile failed for $Source"
    }
}
Write-Output "MCU_HAL_PORT_ARMCLANG_OK=1"
