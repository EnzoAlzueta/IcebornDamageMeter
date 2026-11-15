param([switch]$SkipGraphics)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$cmakeCommand = Get-Command cmake.exe -ErrorAction SilentlyContinue
$cmake = if ($cmakeCommand) { $cmakeCommand.Source } else { Join-Path $env:VSINSTALLDIR 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' }
$ninjaCommand = Get-Command ninja.exe -ErrorAction SilentlyContinue
$ninja = if ($ninjaCommand) { $ninjaCommand.Source } else { Join-Path $env:VSINSTALLDIR 'Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe' }
if (-not (Test-Path -LiteralPath $cmake) -or -not (Test-Path -LiteralPath $ninja)) { throw 'Install CMake tools for Windows in Visual Studio Installer.' }
$preset = if ($SkipGraphics) { 'windows-ci' } else { 'windows-release' }
Push-Location $projectRoot
try {
    & $cmake --preset $preset "-DCMAKE_MAKE_PROGRAM=$ninja"
    if ($LASTEXITCODE) { throw 'CMake configure failed.' }
    & $cmake --build --preset $preset --parallel 2
    if ($LASTEXITCODE) { throw 'Compilation failed.' }
    $ctest = Join-Path (Split-Path $cmake) 'ctest.exe'
    & $ctest --preset $preset
    if ($LASTEXITCODE) { throw 'Tests failed. No package was produced.' }
    $buildRoot = Join-Path $projectRoot $(if ($SkipGraphics) { 'build\cmake-ci' } else { 'build\cmake-release' })
    & (Join-Path $PSScriptRoot 'package.ps1') -BuildRoot $buildRoot
} finally { Pop-Location }
