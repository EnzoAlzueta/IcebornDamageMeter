param([Parameter(Mandatory)][string]$LoaderDll,[Parameter(Mandatory)][string]$BuildRoot)
$ErrorActionPreference = 'Stop'
# Optional local integration evidence. The third-party loader is supplied by the
# developer's existing install; it is never downloaded, vendored or packaged.
# This launches a fixture named MonsterHunterWorld.exe, not the actual game.
$fixture = Join-Path $BuildRoot ('seamless-chain-' + [Guid]::NewGuid().ToString('N'))
$plugins = Join-Path $fixture 'nativePC\plugins'
New-Item -ItemType Directory -Path $plugins -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $BuildRoot 'bootstrap_tests.exe') -Destination (Join-Path $fixture 'MonsterHunterWorld.exe')
Copy-Item -LiteralPath $LoaderDll -Destination (Join-Path $fixture 'xinput9_1_0.dll')
Copy-Item -LiteralPath (Join-Path $BuildRoot 'bootstrap\xinput9_1_0.dll') -Destination (Join-Path $fixture 'xinput9_1_0.chain.dll')
Copy-Item -LiteralPath (Join-Path $BuildRoot 'bootstrap_fixture.dll') -Destination (Join-Path $plugins 'iceborne_damage_meter.dll')
& (Join-Path $fixture 'MonsterHunterWorld.exe') --normal
if ($LASTEXITCODE) { throw "Seamless chain fixture failed: $LASTEXITCODE" }
$log = Join-Path $fixture 'iceborne_damage_meter_bootstrap.log'
if (-not (Test-Path -LiteralPath $log) -or (Get-Content -Raw -LiteralPath $log) -notmatch 'plugin loaded') {
    throw 'Chain did not load the fixture plugin.'
}
Write-Output 'Actual external-loader chain: plugin load, system input forwarding and duplicate-load isolation passed.'
