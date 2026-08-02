param(
    [Parameter(Mandatory = $true)][string]$Source,
    [string]$OutputDir = ".",
    [string]$GccPrefix = "i686-elf"
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
$elf = Join-Path $work "$name.elf"
$bin = Join-Path $work "$name.bin"
$lsp = Join-Path $outDir "$name.lsp"

$acflags = "-ffreestanding", "-nostdlib", "-fno-pic", "-fno-stack-protector", "-I$sdkRoot\include", "-m32", "-O2", "-c"
& "$GccPrefix-gcc" @acflags $src -o $obj
if ($LASTEXITCODE -ne 0) { exit 1 }

& "$GccPrefix-ld" -T "$sdkRoot\app.ld" -o $elf $obj
if ($LASTEXITCODE -ne 0) { exit 1 }

& "$GccPrefix-objcopy" -O binary $elf $bin
$code = [IO.File]::ReadAllBytes($bin)

$nm = & "$GccPrefix-nm" $elf | Select-String " _start$" | Select-Object -First 1
$entry = 0x10
if ($nm -and $nm.ToString() -match '^\s*([0-9a-fA-F]+)') {
    $entry = [Convert]::ToUInt32($Matches[1], 16) - 0x01000000
}

$hdr = [byte[]]::new(16)
$hdr[0] = 0x4C; $hdr[1] = 0x53; $hdr[2] = 0x50; $hdr[3] = 1
$e = [BitConverter]::GetBytes([uint32]$entry); [Array]::Copy($e, 0, $hdr, 4, 4)
$l = [BitConverter]::GetBytes([uint32]$code.Length); [Array]::Copy($l, 0, $hdr, 8, 4)
$b = [BitConverter]::GetBytes([uint32]0); [Array]::Copy($b, 0, $hdr, 12, 4)
$img = $hdr + $code
[IO.File]::WriteAllBytes($lsp, $img)

Remove-Item $work -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "Built $name.lsp ($($img.Length) B) -> $lsp"
