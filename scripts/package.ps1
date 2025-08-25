<#
	Crea archivi ZIP per la consegna.
	- Modalità "minimal": solo i file richiesti (sorgenti C e Makefile).
	- Modalità "full": include anche README e cartella scenes/.
#>
[CmdletBinding()] param(
	[ValidateSet('minimal','full')]
	[string]$Mode = 'minimal'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function New-Zip {
	param([string]$ZipPath, [string[]]$Include)
	if (Test-Path $ZipPath) { Remove-Item $ZipPath -Force }
	$tmp = New-Item -ItemType Directory -Path (Join-Path ([IO.Path]::GetDirectoryName($ZipPath)) ([IO.Path]::GetFileNameWithoutExtension($ZipPath))) -Force
	try {
		foreach ($item in $Include) {
			if (Test-Path $item) {
				Copy-Item -Path $item -Destination $tmp.FullName -Recurse -Force
			}
		}
		Compress-Archive -Path (Join-Path $tmp.FullName '*') -DestinationPath $ZipPath -Force
	} finally {
		Remove-Item $tmp.FullName -Recurse -Force -ErrorAction SilentlyContinue
	}
}

try {
	$root = Split-Path -Parent $PSScriptRoot
	Push-Location $root
	$date = Get-Date -Format 'yyyyMMdd'

	if ($Mode -eq 'minimal') {
		$zip = Join-Path $root ("raytracer-minimal-{0}.zip" -f $date)
		$include = @('main.c','scene.c','scene.h','ppm.c','ppm.h','Makefile')
		New-Zip -ZipPath $zip -Include $include
		Write-Host "Creato: $zip"
	} else {
		$zip = Join-Path $root ("raytracer-full-{0}.zip" -f $date)
		$include = @('main.c','scene.c','scene.h','ppm.c','ppm.h','Makefile','README.md','scenes')
		New-Zip -ZipPath $zip -Include $include
		Write-Host "Creato: $zip"
	}
}
catch {
	Write-Error $_
	exit 1
}
finally { Pop-Location }

