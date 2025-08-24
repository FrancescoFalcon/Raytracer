$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$scenes = Get-ChildItem -Path (Join-Path $root 'scenes') -Filter *.txt
foreach($s in $scenes){
  & (Join-Path $PSScriptRoot 'render.ps1') -ScenePath $s.FullName -Width 1280 -Height 720
}
