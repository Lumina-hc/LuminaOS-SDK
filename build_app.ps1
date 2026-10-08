param(
    [Parameter(Mandatory = $true)][string]$Source,
    [string]$OutputDir = ".",
    [string]$GccPrefix = "x86_64-elf"
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $Source)) { Write-Host "Source not found: $Source" -ForegroundColor Red; exit 1 }
$src = [System.IO.Path]::GetFullPath($Source)
$name = [System.IO.Path]::GetFileNameWithoutExtension($src)
$work = Join-Path $env:TEMP "lsp_build_$PID"
New-Item -ItemType Directory -Path $work -Force | Out-Null

$sdkRoot = $PSScriptRoot
$outDir = [System.IO.Path]::GetFullPath($OutputDir)
New-Item -ItemType Directory -Path $outDir -Force | Out-Null

$obj = Join-Path $work "$name.o"
$elf = Join-Path $outDir "$name.elf"

$acflags = "-ffreestanding", "-nostdlib", "-fno-pic", "-fno-pie", "-fno-stack-protector", "-I$sdkRoot\include", "-m64", "-mno-red-zone", "-mgeneral-regs-only", "-mno-mmx", "-mno-80387", "-Wall", "-O2", "-c"
$prevEap = $ErrorActionPreference
$ErrorActionPreference = "Continue"
& "$GccPrefix-gcc" @acflags $src -o $obj
$gccCode = $LASTEXITCODE
$ErrorActionPreference = $prevEap
if ($gccCode -ne 0) { Write-Host "gcc failed: $Source" -ForegroundColor Red; exit 1 }

$ErrorActionPreference = "Continue"
& "$GccPrefix-ld" -T "$sdkRoot\elf.ld" -o $elf $obj
$ldCode = $LASTEXITCODE
$ErrorActionPreference = $prevEap
if ($ldCode -ne 0) { Write-Host "ld failed: $Source" -ForegroundColor Red; exit 1 }

$elfData = [IO.File]::ReadAllBytes($elf)
Write-Host "Built $name.elf ($($elfData.Length) B) -> $elf"

Remove-Item $work -Recurse -Force -ErrorAction SilentlyContinue
