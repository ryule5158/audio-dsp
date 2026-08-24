Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

& (Join-Path $PSScriptRoot "regenerate_cubemx.ps1")
& (Join-Path $PSScriptRoot "build_keil.ps1")

Write-Host "STM32H743 Audio template verification passed."
