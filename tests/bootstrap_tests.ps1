param([string]$HostExecutable,[string]$BootstrapDll,[string]$PluginFixture,[string]$WorkRoot)
$ErrorActionPreference = 'Stop'
$root = Join-Path $WorkRoot ([Guid]::NewGuid().ToString('N'))
foreach ($mode in @('normal','missing','wrong-host')) {
    $case = Join-Path $root $mode
    $plugins = Join-Path $case 'nativePC\plugins'
    New-Item -ItemType Directory -Path $plugins -Force | Out-Null
    $name = if ($mode -eq 'wrong-host') { 'Unrelated.exe' } else { 'MonsterHunterWorld.exe' }
    $hostPath = Join-Path $case $name
    Copy-Item -LiteralPath $HostExecutable -Destination $hostPath
    Copy-Item -LiteralPath $BootstrapDll -Destination (Join-Path $case 'xinput9_1_0.dll')
    if ($mode -ne 'missing') { Copy-Item -LiteralPath $PluginFixture -Destination (Join-Path $plugins 'iceborne_damage_meter.dll') }
    # Run from a different working directory to catch relative-path loading.
    Push-Location $env:TEMP
    try { & $hostPath "--$mode"; if ($LASTEXITCODE) { throw "Bootstrap case $mode failed: $LASTEXITCODE" } }
    finally { Pop-Location }
}
