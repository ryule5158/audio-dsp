[CmdletBinding()]
param(
    [ValidateSet('selftest', 'wm8960_waveshare', 'i2s_external', 'soc_i2s')]
    [string]$Profile = 'selftest',
    [ValidateSet('create', 'sim', 'synth', 'all')]
    [string]$Action = 'all',
    [string]$Vivado = 'E:\Vivado\2018.3\bin\vivado.bat'
)

$ErrorActionPreference = 'Stop'
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$flow = Join-Path $scriptDir 'vivado\flow.tcl'

if (-not (Test-Path -LiteralPath $Vivado)) {
    throw "Vivado 2018.3 not found at '$Vivado'. Pass -Vivado explicitly."
}

& $Vivado -mode batch -nojournal -nolog -source $flow -tclargs $Profile $Action
if ($LASTEXITCODE -ne 0) {
    throw "Vivado flow failed with exit code $LASTEXITCODE"
}
