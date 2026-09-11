[CmdletBinding()]
param(
    [switch]$CheckRemote,
    [switch]$Json,
    [string]$RepoRoot
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    $RepoRoot = Split-Path -Parent $PSScriptRoot
}
$RepoRoot = (Resolve-Path -LiteralPath $RepoRoot).Path

$worklogPath = Join-Path $RepoRoot 'docs\PROJECT_WORKLOG.md'
$checkpointPath = Join-Path $RepoRoot 'docs\PROJECT_CHECKPOINT.json'

function Get-OptionalProperty {
    param(
        [object]$Object,
        [string]$Name
    )
    if ($null -eq $Object) {
        return $null
    }
    $property = $Object.PSObject.Properties[$Name]
    if ($null -eq $property) {
        return $null
    }
    return $property.Value
}

function Get-FileSha256 {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return $null
    }
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash
}

function Get-WorklogState {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return [ordered]@{
            exists = $false
            path = $Path
            line_count = 0
            last_entry = $null
            last_heading = $null
            max_sequence = $null
            duplicate_sequences = @()
            sha256 = $null
        }
    }

    $lines = @(Get-Content -LiteralPath $Path -Encoding UTF8)
    $entries = New-Object System.Collections.Generic.List[object]
    $sequences = New-Object System.Collections.Generic.List[int]
    foreach ($line in $lines) {
        if ($line -match '^### (W-\d{8}-\d{3})(.*)$') {
            $entryId = $Matches[1]
            $suffix = $Matches[2].Trim()
            $sequence = [int]$entryId.Substring($entryId.Length - 3)
            [void]$entries.Add([ordered]@{
                id = $entryId
                heading = ('### ' + $entryId + $suffix)
            })
            [void]$sequences.Add($sequence)
        }
    }

    $lastEntry = $null
    $lastHeading = $null
    if ($entries.Count -gt 0) {
        $lastEntry = $entries[$entries.Count - 1].id
        $lastHeading = $entries[$entries.Count - 1].heading
    }
    $duplicates = @(
        $sequences |
            Group-Object |
            Where-Object { $_.Count -gt 1 } |
            ForEach-Object { [int]$_.Name }
    )

    return [ordered]@{
        exists = $true
        path = $Path
        line_count = $lines.Count
        last_entry = $lastEntry
        last_heading = $lastHeading
        max_sequence = $(if ($sequences.Count -gt 0) { ($sequences | Measure-Object -Maximum).Maximum } else { $null })
        duplicate_sequences = $duplicates
        sha256 = Get-FileSha256 -Path $Path
    }
}

function Invoke-GitText {
    param([string[]]$Arguments)
    try {
        $outputLines = @(git -C $RepoRoot @Arguments 2>&1 | ForEach-Object { [string]$_ })
        return [ordered]@{
            exit_code = [int]$LASTEXITCODE
            output = ($outputLines -join [Environment]::NewLine)
        }
    } catch {
        return [ordered]@{
            exit_code = -1
            output = $_.Exception.Message
        }
    }
}

function Get-GitState {
    $headResult = Invoke-GitText -Arguments @('rev-parse', 'HEAD')
    $branchResult = Invoke-GitText -Arguments @('symbolic-ref', '--short', '-q', 'HEAD')
    $branchName = $branchResult.output.Trim()
    if ($branchResult.exit_code -ne 0 -or [string]::IsNullOrWhiteSpace($branchName)) {
        $branchName = '(detached or unavailable)'
    }
    $statusResult = Invoke-GitText -Arguments @('status', '--short', '--branch')
    $originResult = Invoke-GitText -Arguments @('remote', 'get-url', 'origin')
    $remoteMain = $null
    if ($CheckRemote) {
        $remoteMain = Invoke-GitText -Arguments @('ls-remote', 'origin', 'refs/heads/main')
    }
    $statusLines = @(
        $statusResult.output -split '\r?\n' |
            Where-Object { $_ -and ($_ -notmatch '^##') }
    )
    return [ordered]@{
        head = $(if ($headResult.exit_code -eq 0) { $headResult.output.Trim() } else { $null })
        branch = $branchName
        origin = $originResult.output.Trim()
        status_exit_code = $statusResult.exit_code
        status = @($statusResult.output -split '\r?\n' | Where-Object { $_ })
        dirty = ($statusLines.Count -gt 0)
        remote_checked = [bool]$CheckRemote
        remote_main = $(if ($null -ne $remoteMain) {
            [ordered]@{
                exit_code = $remoteMain.exit_code
                output = $remoteMain.output.Trim()
            }
        } else {
            $null
        })
    }
}

function Get-KeyPathState {
    $relativePaths = [ordered]@{
        mcu_ioc = 'mcu\project\STM32H743_Audio\STM32H743_Audio.ioc'
        mcu_uvprojx = 'mcu\project\STM32H743_Audio\MDK-ARM\STM32H743_Audio.uvprojx'
        mcu_uvoptx = 'mcu\project\STM32H743_Audio\MDK-ARM\STM32H743_Audio.uvoptx'
        mcu_regenerate = 'mcu\scripts\regenerate_cubemx.ps1'
        mcu_build = 'mcu\scripts\build_keil.ps1'
        fpga_selftest_xpr = 'fpga\projects\audio_dsp_bx71_selftest\audio_dsp_bx71_selftest.xpr'
        fpga_soc_i2s_xpr = 'fpga\projects\audio_dsp_bx71_soc_i2s\audio_dsp_bx71_soc_i2s.xpr'
        fpga_open = 'fpga\open_project.ps1'
        fpga_build = 'fpga\build.ps1'
    }
    $state = [ordered]@{}
    foreach ($key in $relativePaths.Keys) {
        $absolute = Join-Path $RepoRoot $relativePaths[$key]
        $state[$key] = [ordered]@{
            path = $relativePaths[$key].Replace('\', '/')
            exists = (Test-Path -LiteralPath $absolute -PathType Leaf)
        }
    }

    $projectsRoot = Join-Path $RepoRoot 'fpga\projects'
    $xprFiles = @()
    if (Test-Path -LiteralPath $projectsRoot -PathType Container) {
        $xprFiles = @(
            Get-ChildItem -LiteralPath $projectsRoot -Recurse -Filter '*.xpr' -File |
                Sort-Object FullName |
                ForEach-Object {
                    $_.FullName.Substring($RepoRoot.Length + 1).Replace('\', '/')
                }
        )
    }
    return [ordered]@{
        paths = $state
        fpga_native_projects = $xprFiles
    }
}

$worklog = Get-WorklogState -Path $worklogPath
$git = Get-GitState
$keyState = Get-KeyPathState

$checkpoint = $null
$checkpointParseError = $null
if (Test-Path -LiteralPath $checkpointPath -PathType Leaf) {
    try {
        $checkpoint = Get-Content -LiteralPath $checkpointPath -Raw | ConvertFrom-Json
    } catch {
        $checkpointParseError = $_.Exception.Message
    }
}

$staleReasons = New-Object System.Collections.Generic.List[string]
if (-not (Test-Path -LiteralPath $checkpointPath -PathType Leaf)) {
    [void]$staleReasons.Add('checkpoint file is missing')
} elseif ($null -ne $checkpointParseError) {
    [void]$staleReasons.Add('checkpoint JSON failed to parse')
} else {
    $checkpointWorklog = Get-OptionalProperty -Object $checkpoint -Name 'worklog'
    $checkpointGit = Get-OptionalProperty -Object $checkpoint -Name 'git'
    $checkpointLastEntry = Get-OptionalProperty -Object $checkpointWorklog -Name 'last_entry'
    $checkpointHash = Get-OptionalProperty -Object $checkpointWorklog -Name 'sha256'
    $checkpointHead = Get-OptionalProperty -Object $checkpointGit -Name 'head'
    if ([string]$checkpointLastEntry -ne [string]$worklog.last_entry) {
        [void]$staleReasons.Add('worklog last_entry differs from checkpoint')
    }
    if ([string]$checkpointHash -ne [string]$worklog.sha256) {
        [void]$staleReasons.Add('worklog SHA-256 differs from checkpoint')
    }
    if ([string]$checkpointHead -ne [string]$git.head) {
        [void]$staleReasons.Add('local HEAD differs from checkpoint')
    }
}

$stage = Get-OptionalProperty -Object $checkpoint -Name 'stage'
$nextAction = Get-OptionalProperty -Object $stage -Name 'next_action'
if ([string]::IsNullOrWhiteSpace([string]$nextAction)) {
    $nextAction = 'Read the worklog and determine the next recorded gate.'
}

$result = [ordered]@{
    generated_at = (Get-Date).ToString('o')
    repo_root = $RepoRoot
    scope = [ordered]@{
        active = @('mcu_h743iit6_cubemx_keil', 'fpga_bx71_zynq7020_vivado2018_xsdk')
        frozen = @('mpu_imx6ull', 'ubuntu_vm', 'gitee_login')
    }
    worklog = $worklog
    checkpoint = [ordered]@{
        path = $checkpointPath
        exists = (Test-Path -LiteralPath $checkpointPath -PathType Leaf)
        parse_error = $checkpointParseError
        stale = ($staleReasons.Count -gt 0)
        stale_reasons = @($staleReasons)
    }
    git = $git
    key_paths = $keyState.paths
    fpga_native_projects = $keyState.fpga_native_projects
    next_action = [string]$nextAction
    verification_boundary = @(
        'This command is read-only by design and does not modify the repository, remote, tools, or hardware.',
        'A present native project is not evidence of a successful synthesis, software build, bitstream, or board test.',
        'A stale checkpoint is an explicit recovery signal after later worklog appends or commits.'
    )
}

if ($Json) {
    $result | ConvertTo-Json -Depth 12
    exit 0
}

Write-Output 'AUDIO_DSP RESUME CONTEXT'
Write-Output ('Repository: ' + $RepoRoot)
Write-Output ('Scope: MCU H743IIT6 + FPGA BX71/Zynq-7020; frozen: MPU/VM/Gitee login')
Write-Output ('Worklog: exists=' + $worklog.exists + ', last=' + $worklog.last_entry + ', lines=' + $worklog.line_count + ', sha256=' + $worklog.sha256)
Write-Output ('Worklog duplicate sequences: ' + (@($worklog.duplicate_sequences) -join ', '))
Write-Output ('Checkpoint: exists=' + (Test-Path -LiteralPath $checkpointPath -PathType Leaf) + ', stale=' + ($staleReasons.Count -gt 0))
if ($staleReasons.Count -gt 0) {
    foreach ($reason in $staleReasons) {
        Write-Output ('  stale reason: ' + $reason)
    }
}
Write-Output ('Git: branch=' + $git.branch + ', head=' + $git.head + ', dirty=' + $git.dirty)
Write-Output ('FPGA native XPR files: ' + (@($keyState.fpga_native_projects) -join ', '))
Write-Output ('Next action: ' + $nextAction)
Write-Output 'Boundary: read-only status; no claim of CubeMX, Keil, Vivado, XSDK, synthesis, bitstream, or hardware success.'
