# Iceborn Damage Meter

A compact, in-process damage meter for **Monster Hunter World: Iceborne** on
Windows x64. No separate overlay application or translation service is required.

**Public beta: 0.6.11.** Download the installable mod from
[Nexus Mods](https://www.nexusmods.com/monsterhunterworld/mods/8969).
The `r2` package revision removes development documents from the runtime ZIP
and provides current installation guides; the tested DLLs and default settings
are unchanged. This repository contains the source code and developer tools.
The interface supports 13 languages. This remains experimental pre-1.0 software
with limited field coverage.

**Development candidate: 0.6.12.** Adds an optional cumulative binary hunt archive
for a future external analyzer. The UI still keeps ten hunts. Storage is handled
in the background with bounded memory and startup reads; no historical analysis
runs in-game. See [archive format and operation](docs/HUNT-ARCHIVE.md). This
candidate still requires real-game validation before replacing the public beta.

[Installation](packaging/INSTALL.md) · [Español](packaging/INSTALL.es.md) ·
[Compatibility](docs/COMPATIBILITY.md) · [Validation](docs/VALIDATION.md) ·
[Build](docs/BUILDING.md) · [Changelog](CHANGELOG.md)

## Features

- Player damage, team share and observed DPS, with native weapon icons.
- Compact panel, live details, last hunt and ten-hunt history.
- Mission names/objectives when available; generic investigations labeled clearly.
- Scale 40–175%, width 260–640, opacity, visible rows and configurable controls.
- Manual damage sharing through paced keyboard input or experimental direct chat;
  explicit Copy summary alternative. Full names are kept when each entry fits.
- Automatic game-language detection, with offline translations in English,
  Spanish, Portuguese, French, German, Italian, Japanese, Korean, Simplified and
  Traditional Chinese, Russian, Polish and Arabic.

## Install

Close the game and open its local files from Steam.

| Setup | Package and files |
|---|---|
| No loader | Normal ZIP: nativePC + xinput9_1_0.dll |
| Stracker | Normal ZIP: only nativePC |

Never replace another mod's root or chain DLL. On update, preserve your INI and
hunt history. See [the installation guide](packaging/INSTALL.md)
for complete language/chat settings, updates, removal and troubleshooting.

## Default controls

| Key | Action |
|---|---|
| F8 | Show/hide |
| Ctrl+F8 | Synthetic preview |
| Ctrl+F6 | Menu |
| F6 | Settings |
| F7 | History |
| Ctrl+F9 | Unlock/lock dragging |

Existing key bindings are preserved.

## Requirements and limits

Windows 10/11 x64, DirectX 11 or 12, SDR and the supported executable for memory
profile 421810. Unsupported executable hashes disable damage reading.

**Expedition and Guiding Lands damage are not supported.** Those contexts show
no data rather than reusing a previous mission. Multi-target quests show mission
totals, not per-monster attribution. Investigations sharing generic ID 30000 are
excluded from personal DPS comparisons. Missing quest translations use English.

DPS excludes damage already present at first observation and includes time spent
present, including pauses and travel. It is not active attack time or a normalized
skill score. See [statistics](docs/METRICS.md).

The author and a friend tested 0.6.11 in-game and reported no further crashes.
The release reduces repeated DX11 UI raster work, suspends drawing on focus loss
or minimize and adds graphics diagnostics. The original ERR12 cause is not
confirmed. Not every language, party size, GPU or loader combination has been
independently validated. Direct chat remains optional and experimental;
successful input processing is not proof of reception.

## Build and contribute

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
```

Requires Visual Studio 2022 or newer C++ Build Tools, Windows SDK, CMake and Ninja.
Use `-SkipGraphics` for non-graphics CI. Local DX11/DX12 tests are separate from
real-game tests. GitHub Actions runs the non-graphics build and tests; check its
actual run result before treating a commit as verified by hosted CI.
See [BUILDING](docs/BUILDING.md), [CONTRIBUTING](CONTRIBUTING.md) and
[localization](locales/README.md). Use English for primary project material;
Spanish companion guides use `.es.md` filenames.

Counters are sampled at 4 Hz, text/composition are cached, and persistence uses
a background worker. History/player storage is bounded. No measured performance
claim applies universally to other systems.

## License and notices

Original project code is MIT; see [LICENSE](LICENSE). Dependencies and technical
references are listed in [THIRD_PARTY.md](THIRD_PARTY.md). Extracted weapon artwork
is copyright CAPCOM and is separate from the code license; attribution does not
grant additional redistribution rights. No third-party loaders,
full game atlases or extraction tools are bundled. The bootstrap is original
project code, not a copy of a loader or Microsoft's XInput DLL.

## AI disclosure

OpenAI Codex was used to generate and revise code, interface translations,
documentation, mod-page text and HTML/CSS promotional layouts. The author directed
features and adjustments and tested the mod in-game with another player.
UI preview images are diagnostic renders with labeled sample data, not generated
gameplay screenshots. Original CAPCOM weapon icons are not AI-generated artwork.
