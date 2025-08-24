<#
	Simple runner for the raytracer.
	- Defaults to scenes/scene_pdf_example.txt at 640x360
	- Uses scripts/render.ps1 if available, else calls the executable directly
#>
[CmdletBinding()]
param(
	[string]$Scene = "./scenes/scene_pdf_example.txt",
	[int]$Width = 640,
	[int]$Height = 360
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Ensure-Dirs {
	param([string[]]$Paths)
	foreach ($p in $Paths) { if (-not (Test-Path $p)) { New-Item -ItemType Directory -Path $p | Out-Null } }
}

function Resolve-ScenePath {
	param([string]$Path)
	if (Test-Path $Path) { return (Resolve-Path $Path).Path }
	$alt = Join-Path -Path (Join-Path -Path $PSScriptRoot -ChildPath 'scenes') -ChildPath ([IO.Path]::GetFileName($Path))
	if (Test-Path $alt) { return (Resolve-Path $alt).Path }
	throw "Scene file not found: $Path"
}

try {
	$scenePath = Resolve-ScenePath -Path $Scene
	Ensure-Dirs -Paths @("$PSScriptRoot/ppms", "$PSScriptRoot/renders")

	$renderScript = Join-Path $PSScriptRoot 'scripts/render.ps1'
	if (Test-Path $renderScript -PathType Leaf -and ((Get-Item $renderScript).Length -gt 0)) {
		& $renderScript -ScenePath $scenePath -Width $Width -Height $Height
		exit $LASTEXITCODE
	}

	# Fallback: call the executable directly
	$exe = Join-Path $PSScriptRoot 'raytracer.exe'
	if (-not (Test-Path $exe)) {
		Write-Warning "raytracer.exe non trovato. Compila con MSYS2: mingw32-make (vedi README)."
		exit 1
	}

	$base = [IO.Path]::GetFileNameWithoutExtension($scenePath)
	$stamp = "{0}x{1}" -f $Width,$Height
	$ts = Get-Date -Format 'yyyyMMdd_HHmmss'
	$ppm = Join-Path $PSScriptRoot ("ppms/{0}-{1}-{2}.ppm" -f $base,$stamp,$ts)

	Write-Host "Running: $($exe) $scenePath $ppm $Width $Height"
	& $exe $scenePath $ppm $Width $Height
	if ($LASTEXITCODE -ne 0) { throw "raytracer.exe exited with code $LASTEXITCODE" }
	Write-Host "PPM scritto: $ppm"
}
catch {
	Write-Error $_
	exit 1
}

