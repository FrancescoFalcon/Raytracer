<#
	Setup rapido su Windows: installa MSYS2, toolchain MinGW e compila il progetto.
	Richiede privilegi per installare MSYS2 via winget.
#>
[CmdletBinding()] param(
	[switch]$InstallMsys2,
	[switch]$InstallToolchain,
	[switch]$Build
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Invoke-Cmd {
	param([string]$Cmd)
	Write-Host "$Cmd"
	iex $Cmd
}

try {
	if ($InstallMsys2) {
		Invoke-Cmd 'winget install -e --id MSYS2.MSYS2 --accept-package-agreements --accept-source-agreements'
	}

	if ($InstallToolchain) {
		$bash = 'C:\\msys64\\usr\\bin\\bash.exe'
		if (-not (Test-Path $bash)) { throw 'MSYS2 non trovato. Esegui con -InstallMsys2.' }
		& $bash -lc "pacman -Sy --noconfirm && pacman -S --needed --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-libwinpthread-git"
	}

	if ($Build) {
		$bash = 'C:\\msys64\\usr\\bin\\bash.exe'
		if (-not (Test-Path $bash)) { throw 'MSYS2 non trovato. Esegui con -InstallMsys2.' }
		$proj = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
		& $bash -lc "cd $(wslpath -a -u $proj 2>$null); PATH=/mingw64/bin:$PATH mingw32-make -j" 2>$null
		if ($LASTEXITCODE -ne 0) {
			# fallback senza wslpath
			& $bash -lc "cd /c/Users/susid/OneDrive/Desktop/skuola/Raytracer && PATH=/mingw64/bin:$PATH mingw32-make -j"
		}
	}
}
catch {
	Write-Error $_
	exit 1
}

