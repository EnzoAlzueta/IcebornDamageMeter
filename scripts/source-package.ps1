param([ValidateRange(0, 2147483647)][int]$PackageRevision = 0)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$cmake = [IO.File]::ReadAllText((Join-Path $projectRoot 'CMakeLists.txt'))
$version = [regex]::Match($cmake,'project\(IceborneDamageMeter VERSION ([0-9.]+)').Groups[1].Value
$packageVersion = if ($PackageRevision -gt 0) { "$version-r$PackageRevision" } else { $version }
$dist = Join-Path $projectRoot 'dist'
New-Item -ItemType Directory -Path $dist -Force | Out-Null
$files = @('src','assets','locales','bootstrap','third_party','tests','scripts','packaging','docs','.github',
    '.gitignore','.gitattributes','.editorconfig','CMakeLists.txt','CMakePresets.json','build.ps1','build-native.cmd',
    'iceborne_damage_meter.ini',
    'README.md','INSTALL.md','INSTALL.es.md','LICENSE','THIRD_PARTY.md','THIRD_PARTY.es.md','CHANGELOG.md','CONTRIBUTING.md')
Add-Type -AssemblyName System.IO.Compression,System.IO.Compression.FileSystem
$zipPath = Join-Path $dist "IceborneDamageMeter-$packageVersion-source.zip"
if (Test-Path -LiteralPath $zipPath) { Remove-Item -LiteralPath $zipPath }
$archive = [IO.Compression.ZipFile]::Open($zipPath,[IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($entry in $files) {
        $path = Join-Path $projectRoot $entry
        $items = if (Test-Path -LiteralPath $path -PathType Container) { @(Get-ChildItem -LiteralPath $path -Recurse -File -Force) } else { @(Get-Item -LiteralPath $path -Force) }
        foreach ($file in $items) {
            $relative = $file.FullName.Substring($projectRoot.Length+1).Replace('\','/')
            [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive,$file.FullName,$relative,[IO.Compression.CompressionLevel]::Optimal) | Out-Null
        }
    }
} finally { $archive.Dispose() }
$checksumPath = Join-Path $dist "IceborneDamageMeter-$packageVersion-SHA256SUMS.txt"
$lines = @()
if (Test-Path -LiteralPath $checksumPath) { $lines = @([IO.File]::ReadAllLines($checksumPath) | Where-Object { $_ -and $_ -notlike '*-source.zip' }) }
$lines += (Get-FileHash -LiteralPath $zipPath -Algorithm SHA256).Hash.ToLowerInvariant() + ' *' + (Split-Path -Leaf $zipPath)
[IO.File]::WriteAllText($checksumPath,($lines -join "`n")+"`n",(New-Object Text.UTF8Encoding($false)))
Write-Output "Source package: $zipPath"
