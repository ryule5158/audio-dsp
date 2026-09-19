param(
    [string]$Xsct = 'E:\SDK\2018.3\bin\xsct.bat',
    [ValidateSet('audio')]
    [string]$Profile = 'audio',
    [switch]$DetailedLog
)

$ErrorActionPreference = 'Stop'
$fpgaRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$script = Join-Path $fpgaRoot 'software\build_xsdk.tcl'
$hdf = Join-Path $fpgaRoot 'build\output\audio_dsp_bx71_soc_i2s.hdf'
$sdkRoot = Split-Path -Parent (Split-Path -Parent $Xsct)
$makeDirectory = Join-Path $sdkRoot 'gnuwin\bin'
$toolchainDirectory = Join-Path $sdkRoot 'gnu\aarch32\nt\gcc-arm-none-eabi\bin'
$configuration = Join-Path $fpgaRoot ('build\xsct_configuration_' + $Profile)

if (-not (Test-Path -LiteralPath $Xsct -PathType Leaf)) {
    throw "XSDK 2018.3 XSCT was not found at $Xsct"
}
if (-not (Test-Path -LiteralPath $hdf -PathType Leaf)) {
    throw "Missing $hdf. Run .\build.ps1 soc_i2s all first."
}
if (-not (Test-Path -LiteralPath (Join-Path $makeDirectory 'make.exe') -PathType Leaf)) {
    throw "XSDK GNU make was not found below $sdkRoot"
}
if (-not (Test-Path -LiteralPath (Join-Path $toolchainDirectory 'arm-none-eabi-readelf.exe') -PathType Leaf)) {
    throw "XSDK ARM readelf was not found below $sdkRoot"
}

New-Item -ItemType Directory -Force -Path $configuration | Out-Null
$configurationUri = 'file:/' + $configuration.Replace('\', '/')
$savedJavaOptions = $env:JAVA_TOOL_OPTIONS
$savedPath = $env:Path
$savedXsdkMake = $env:AUDIO_DSP_XSDK_MAKE
$savedXsdkReadelf = $env:AUDIO_DSP_XSDK_READELF
$env:JAVA_TOOL_OPTIONS = (($savedJavaOptions +
    " -Dosgi.configuration.area=$configurationUri").Trim())
$env:Path = $makeDirectory + ';' + $toolchainDirectory + ';' + $savedPath
$env:AUDIO_DSP_XSDK_MAKE = Join-Path $makeDirectory 'make.exe'
$env:AUDIO_DSP_XSDK_READELF = Join-Path $toolchainDirectory 'arm-none-eabi-readelf.exe'

try {
    $xsctOutput = & $Xsct $script $Profile 2>&1
    $exitCode = $LASTEXITCODE
    $logPath = Join-Path $fpgaRoot ('build\xsdk_' + $Profile + '_build.log')
    $xsctOutput | Set-Content -LiteralPath $logPath -Encoding UTF8
    if ($DetailedLog) {
        $xsctOutput | ForEach-Object { Write-Host $_ }
    } else {
        $xsctOutput | Select-Object -Last 12 | ForEach-Object { Write-Host $_ }
        Write-Host "Full XSDK build log: $logPath"
    }
} finally {
    $env:JAVA_TOOL_OPTIONS = $savedJavaOptions
    $env:Path = $savedPath
    $env:AUDIO_DSP_XSDK_MAKE = $savedXsdkMake
    $env:AUDIO_DSP_XSDK_READELF = $savedXsdkReadelf
}

if (($exitCode -ne 0) -or
    (($xsctOutput -join "`n") -notmatch 'FPGA_XSDK_BUILD_OK')) {
    throw "XSDK/XSCT build did not produce its success marker (exit $exitCode)"
}
