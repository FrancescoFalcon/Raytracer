<#
	Sposta eventuali file di scena (.txt) presenti per errore nella root del repo in ./scenes/.
	- Riconosce i file scena cercando tag come "VP ", "OBJ_N", "S ".
	- Evita duplicati usando hash: se identico a un file già presente in scenes/, elimina l'originale.
	- Se esiste un file diverso con lo stesso nome, aggiunge un suffisso numerico.
	- Opzione -DryRun per vedere le azioni senza eseguirle.
#>
[CmdletBinding()]
param(
	[switch]$DryRun
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Ensure-Dir {
	param([string]$Path)
	if (-not (Test-Path $Path)) { New-Item -ItemType Directory -Path $Path | Out-Null }
}

function Get-HashHex {
	param([string]$Path)
	return (Get-FileHash -Algorithm SHA256 -Path $Path).Hash
}

try {
	$root = Split-Path -Parent $PSScriptRoot
	$scenesDir = Join-Path $root 'scenes'
	Ensure-Dir $scenesDir

	$candidates = Get-ChildItem -Path $root -Filter *.txt -File | Where-Object { $_.DirectoryName -ne $scenesDir }
	if (-not $candidates) { Write-Host 'Nessun file .txt da analizzare nella root.'; return }

	foreach ($f in $candidates) {
		$head = @()
		try { $head = Get-Content -Path $f.FullName -TotalCount 50 -ErrorAction Stop } catch { }
		$isScene = $false
		foreach ($line in $head) {
			if ($line -match '^\s*(VP|BG|OBJ_N|S)\b') { $isScene = $true; break }
		}
		if (-not $isScene) {
			Write-Host "Ignoro (non sembra un file scena): $($f.Name)"
			continue
		}

		$target = Join-Path $scenesDir $f.Name
		if (Test-Path $target) {
			$srcHash = Get-HashHex $f.FullName
			$dstHash = Get-HashHex $target
			if ($srcHash -eq $dstHash) {
				if ($DryRun) { Write-Host "[DRY] Duplicato identico, elimino: $($f.Name)" }
				else { Remove-Item -Path $f.FullName; Write-Host "Eliminato duplicato: $($f.Name)" }
				continue
			} else {
				$base = [IO.Path]::GetFileNameWithoutExtension($f.Name)
				$ext = $f.Extension
				$i = 1
				do {
					$target = Join-Path $scenesDir ("{0}({1}){2}" -f $base,$i,$ext)
					$i++
				} while (Test-Path $target)
			}
		}

		if ($DryRun) { Write-Host "[DRY] Sposto $($f.Name) -> $([IO.Path]::GetFileName($target))" }
		else { Move-Item -Path $f.FullName -Destination $target; Write-Host "Spostato: $($f.Name) -> $([IO.Path]::GetFileName($target))" }
	}
}
catch {
	Write-Error $_
	exit 1
}

