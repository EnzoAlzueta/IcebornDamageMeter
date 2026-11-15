param(
    [Parameter(Mandatory)][string]$BuildRoot,
    [ValidateRange(0, 2147483647)][int]$PackageRevision = 0
)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$manifest = Get-Content -Raw -LiteralPath (Join-Path $BuildRoot 'manifest.json') | ConvertFrom-Json
$version = $manifest.version
$packageVersion = if ($PackageRevision -gt 0) { "$version-r$PackageRevision" } else { $version }
if ($PackageRevision -gt 0) {
    $manifest | Add-Member -NotePropertyName packagingRevision -NotePropertyValue $PackageRevision -Force
}
$dist = Join-Path $projectRoot 'dist'
New-Item -ItemType Directory -Path $dist -Force | Out-Null
$checksums = @()
$variants = @(
    @{suffix='windows-x64';bootstrap='xinput9_1_0.dll';guide='README-IceborneDamageMeter.txt'},
    @{suffix='Seamless-windows-x64';bootstrap='xinput9_1_0.chain.dll';guide='README-Seamless.txt'}
)
foreach ($variant in $variants) {
# Stage in a fresh directory to exclude documents left over from older packages.
$packageName = "IceborneDamageMeter-$packageVersion-" + $variant.suffix
$releaseRoot = Join-Path $projectRoot ("build\packages\$packageName-" + [guid]::NewGuid().ToString('N'))
$pluginRoot = Join-Path $releaseRoot 'nativePC\plugins'
New-Item -ItemType Directory -Path $pluginRoot -Force | Out-Null
$dll = Join-Path $BuildRoot 'bin\iceborne_damage_meter.dll'
$bootstrap = Join-Path $BuildRoot 'bootstrap\xinput9_1_0.dll'
if (-not (Test-Path -LiteralPath $bootstrap)) { throw 'Missing compiled bootstrap DLL.' }
$manifest | Add-Member -NotePropertyName bootstrapSha256 -NotePropertyValue (Get-FileHash -LiteralPath $bootstrap -Algorithm SHA256).Hash.ToLowerInvariant() -Force
$manifest.bootstrap = $variant.bootstrap
Copy-Item -LiteralPath $bootstrap -Destination (Join-Path $releaseRoot $variant.bootstrap) -Force
if (-not (Test-Path -LiteralPath $dll)) { throw 'Missing compiled DLL.' }
$manifest | Add-Member -NotePropertyName dllSha256 -NotePropertyValue (Get-FileHash -LiteralPath $dll -Algorithm SHA256).Hash.ToLowerInvariant() -Force
Copy-Item -LiteralPath $dll -Destination $pluginRoot -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'iceborne_damage_meter.ini') -Destination $pluginRoot -Force
$documentationRoot = Join-Path $releaseRoot 'IceborneDamageMeter'
New-Item -ItemType Directory -Path $documentationRoot -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $documentationRoot 'licenses') -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $projectRoot 'third_party\minhook\LICENSE.txt') -Destination (Join-Path $documentationRoot 'licenses\MinHook-LICENSE.txt') -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'third_party\mhworlddata\LICENSE.txt') -Destination (Join-Path $documentationRoot 'licenses\MHWorldData-LICENSE.txt') -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'third_party\mhw-toolkit\LICENSE.txt') -Destination (Join-Path $documentationRoot 'licenses\mhw-toolkit-REFERENCE-LICENSE.txt') -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'assets\Game-artwork-NOTICE.txt') -Destination (Join-Path $documentationRoot 'licenses\Game-artwork-NOTICE.txt') -Force
Copy-Item -LiteralPath (Join-Path $projectRoot ('packaging\' + $variant.guide)) -Destination (Join-Path $releaseRoot 'README-IceborneDamageMeter.txt') -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'LICENSE') -Destination $documentationRoot -Force
foreach ($file in @('INSTALL.md','INSTALL.es.md','THIRD_PARTY.md')) {
    Copy-Item -LiteralPath (Join-Path $projectRoot "packaging\$file") -Destination $documentationRoot -Force
}
[IO.File]::WriteAllText((Join-Path $documentationRoot 'manifest.json'),($manifest | ConvertTo-Json -Depth 5),(New-Object Text.UTF8Encoding($false)))
$zip = Join-Path $dist ($packageName + '.zip')
Compress-Archive -Path (Join-Path $releaseRoot '*') -DestinationPath $zip -Force
$checksums += (Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant() + ' *' + (Split-Path -Leaf $zip)
Write-Output "Package: $zip"
}
[IO.File]::WriteAllText((Join-Path $dist "IceborneDamageMeter-$packageVersion-SHA256SUMS.txt"),($checksums -join "`n")+"`n",(New-Object Text.UTF8Encoding($false)))
