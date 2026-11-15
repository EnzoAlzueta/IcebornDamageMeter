# Building and publishing

Localization data lives in `locales/strings.json`. Regenerate and audit with
`python scripts/check-localization.py`; regenerate the offline quest catalog with
`python scripts/generate-quest-catalog.py`. Generated headers are included so
players and normal C++ builds need no Python or translation service.

English is the source language, with 12 translation columns. See
[localization](../locales/README.md). Public guides start at [INSTALL.md](../INSTALL.md);
Spanish translations use `.es.md`. Generated headers must be committed with their
source JSON/CSV. Changing the project's source language does not change locale
selection or the persistent settings/history formats.

## Toolchain

Windows x64, Visual Studio 2022 or newer with Desktop development with C++, a
Windows 10/11 SDK, and CMake tools for Windows (CMake >= 3.24 and Ninja).
No network dependency download, .NET runtime or HunterPie installation is required
to build the plugin. The MSVC runtime is linked statically.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
```

The wrapper finds Visual Studio with vswhere, initializes its x64 environment,
then uses the checked-in CMake preset. `-SkipGraphics` selects the CI preset and
runs non-graphics tests only. It never launches the actual game. A small isolated
test executable is named MonsterHunterWorld.exe to exercise bootstrap host checks.

From an x64 Developer Prompt you may invoke CMake directly:

```text
cmake --preset windows-release
cmake --build --preset windows-release
ctest --preset windows-release
```

`windows-ci` disables DX11/DX12 tests that need a graphics device. A passing CI build
does not mean those tests or real-game tests passed. Local graphics tests use a
hidden window, synthetic data and WARP/hardware presentation.

## Artifacts

- `dist/IceborneDamageMeter-VERSION-windows-x64.zip`: installable mod.
- `dist/IceborneDamageMeter-VERSION-Seamless-windows-x64.zip`: existing Seamless
  installation; bootstrap is named xinput9_1_0.chain.dll, not the root proxy name.
- `dist/IceborneDamageMeter-VERSION-SHA256SUMS.txt`: archive integrity checksum.
- `IceborneDamageMeter/manifest.json` inside the runtime ZIP: version, platform, supported game hash,
  compatible loaders and both plugin/bootstrap DLL checksums.
- `scripts/source-package.ps1`: creates a source ZIP from an explicit allowlist.

The version is defined in CMakeLists.txt. CMake generates the runtime header, DLL
Windows version resource and manifest from it. Do not edit those generated files.
The supported game hash is deliberately pinned to the matching memory profile;
changing the hash without validating the offsets is not a port to another build.

## GitHub

Create an empty repository named `IceborneDamageMeter`, then push the SOURCE
files, including `.github`, not the runtime ZIP as the repository contents:

```text
git init
git add .
git commit -m "Prepare public source distribution"
git branch -M main
git remote add origin https://github.com/YOUR-ACCOUNT/IceborneDamageMeter.git
git push -u origin main
```

GitHub Actions runs non-graphics tests on Windows and attaches the runtime ZIP
and checksum as workflow artifacts. This workflow has read-only repository
permissions and does not create a public release. After reviewing a build, create
a GitHub Release with a matching version tag (for example v0.6.1) and attach the
runtime ZIP and checksum. GitHub also supplies source archives for tags.

The first hosted workflow run must still be verified after the repository is
created. A prepared workflow is not evidence that it has already run on GitHub.

## Package contents and revisions

Runtime ZIPs use an explicit documentation list from `packaging/`: current
English/Spanish installation and usage guides, credits, licenses and a manifest.
Development guides, contribution instructions, release plans, validation reports
and localization-maintenance notes belong in the source ZIP only. Each runtime
package uses a fresh staging directory under `build/packages` to prevent stale
documents from reappearing.

For a documentation-only repack of an existing binary version, pass
`-PackageRevision 2` to both `scripts/package.ps1 -BuildRoot BUILD_ROOT` and
`scripts/source-package.ps1`. This creates distinct `VERSION-r2` archive/checksum
names; the DLL version remains unchanged. For a published release, use the exact
tested DLLs and verify their hashes against the preceding package. Do not replace
the original frozen archives or claim a code fix for a packaging-only revision.

## Bootstrap

`bootstrap/` contains the original MIT-licensed XInput pass-through and its export
definition. CMake builds it with a static MSVC runtime; the release packages it
as `xinput9_1_0.dll` at the ZIP root, beside nativePC. Users without a loader copy
both to the game folder. Stracker users copy only nativePC. Seamless users copy
nativePC and the chain DLL from the Seamless ZIP, preserving its root proxy.
No installation scripts are shipped or required.
Do not substitute a Windows system DLL or a third-party mod DLL.
The source ZIP includes bootstrap source, resources and isolated fixture tests.

An optional local integration check uses a developer-supplied Seamless loader:
`tests/seamless_chain_smoke.ps1 -LoaderDll PATH -BuildRoot build/cmake-release`.
It runs an isolated fixture, never the game. No third-party loader binary is
downloaded, committed, shipped or required by CI.
