param([switch]$SkipGraphics)
$ErrorActionPreference = 'Stop'
$arguments = @()
if ($SkipGraphics) { $arguments += '-SkipGraphics' }
& (Join-Path $PSScriptRoot 'build-native.cmd') @arguments
if ($LASTEXITCODE) { throw 'Build, validation or packaging failed. See the output above.' }
