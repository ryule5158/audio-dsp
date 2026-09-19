[CmdletBinding()]
param(
    [ValidateSet('selftest', 'soc_i2s')]
    [string]$Profile = 'selftest',
    [string]$Vivado = 'E:\Vivado\2018.3\bin\vivado.bat'
)

$ErrorActionPreference = 'Stop'
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectName = "audio_dsp_bx71_$Profile"
$xpr = Join-Path $scriptDir "projects\$projectName\$projectName.xpr"

if (-not (Test-Path -LiteralPath $Vivado)) {
    throw "Vivado 2018.3 not found at '$Vivado'. Pass -Vivado explicitly."
}
if (-not (Test-Path -LiteralPath $xpr)) {
    throw "Checked-in native project not found: $xpr"
}
& $Vivado $xpr
if ($LASTEXITCODE -ne 0) {
    throw "Vivado exited with code $LASTEXITCODE"
}
