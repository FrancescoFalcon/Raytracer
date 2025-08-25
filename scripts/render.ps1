<#
	Rende una scena con l'eseguibile raytracer e organizza gli output.
	- Scrive il PPM in ./ppms/
	- Se ImageMagick è installato, converte in PNG in ./renders/
#>
[CmdletBinding()] param(
	[Parameter(Mandatory=$true)] [string]$ScenePath,
	[int]$Width = 1280,
	[int]$Height = 720,
	[switch]$NoPng
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Ensure-Dirs {
	param([string[]]$Paths)
	foreach ($p in $Paths) { if (-not (Test-Path $p)) { New-Item -ItemType Directory -Path $p | Out-Null } }
}

function Resolve-Exe {
	$exe = Join-Path $PSScriptRoot '..' | Join-Path -ChildPath 'raytracer.exe'
	if (Test-Path $exe) { return (Resolve-Path $exe).Path }
	throw 'raytracer.exe non trovato. Compila con mingw32-make (vedi README).'
}

function Ensure-MinGWPath {
	$mingw = 'C:\msys64\mingw64\bin'
	if (Test-Path $mingw) {
		if (-not ($env:Path -split ';' | Where-Object { $_ -ieq $mingw })) {
			$env:Path = "$mingw;$env:Path"
			Write-Host "PATH aggiornato con $mingw"
		}
	}
}

try {
	$scene = if (Test-Path $ScenePath) { (Resolve-Path $ScenePath).Path } else { throw "Scene non trovata: $ScenePath" }
	Ensure-Dirs -Paths @("$PSScriptRoot/../ppms", "$PSScriptRoot/../renders")

	$exe = Resolve-Exe
	Ensure-MinGWPath
	$base = [IO.Path]::GetFileNameWithoutExtension($scene)
	$stamp = "{0}x{1}" -f $Width,$Height
	$ts = Get-Date -Format 'yyyyMMdd_HHmmss'
	$ppm = Join-Path $PSScriptRoot ("../ppms/{0}-{1}-{2}.ppm" -f $base,$stamp,$ts)

	Write-Host "Esecuzione: $exe $scene $ppm $Width $Height"
	& $exe $scene $ppm $Width $Height
	if ($LASTEXITCODE -ne 0) { throw "raytracer.exe exited with code $LASTEXITCODE" }
	Write-Host "PPM scritto: $ppm"

	if (-not $NoPng) {
		$magick = Get-Command magick -ErrorAction SilentlyContinue
		if ($magick) {
			$png = Join-Path $PSScriptRoot ("../renders/{0}-{1}-{2}.png" -f $base,$stamp,$ts)
			& $magick $ppm $png
			if ($LASTEXITCODE -ne 0) { Write-Warning 'Conversione PNG fallita.' } else { Write-Host "PNG scritto: $png" }
		} else {
			Write-Host 'ImageMagick non trovato (comando magick). Salto conversione PNG.'
		}
	}
}
catch {
	Write-Error $_
	exit 1
}

