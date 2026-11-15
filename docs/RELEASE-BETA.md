# Iceborne Damage Meter 0.6.11 — prepared public beta

[Español](RELEASE-BETA.es.md) · [Installation](../INSTALL.md)

An integrated damage meter for Monster Hunter World: Iceborne on Windows x64.
Displays party damage, team share, observed DPS and recent hunts, with adjustable
size/position and automatic language selection from the game's Steam settings.

## Included

- Compact panel, mission details and ten-hunt history.
- Mission names/objectives where cataloged; generic investigation labels.
- Full player names when each entry fits; bounded chat messages and Copy summary.
- Optional experimental direct chat without simulated typing.
- Corrected native weapon icons, including Bow and Light Bowgun.
- English primary source/docs and interface translations in 13 languages total.
- Automatic DX11/DX12 selection with persistent settings and history.

## Known limits

No expedition or Guiding Lands damage measurement. Supported executable/profile
421810 only, Windows x64, SDR. Missing quest translations fall back to English;
Simplified Chinese quest titles are not in the upstream catalog. DPS describes
observed participation including pauses/travel, not normalized skill. Generic
investigations are not compared as identical hunts. Not all languages, parties,
GPUs and loader combinations have real-game evidence.

46 local tests passed. On 2026-11-15, the user reported successful 0.6.11 testing
with a friend and no further crashes. The original ERR12 cause is not confirmed.
0.6.11 reduces repeated DX11 UI raster work, suspends drawing on focus loss or
minimize and adds graphics diagnostics. Do not treat successful chat request
processing as delivery confirmation.

## Installation and reporting

Follow [INSTALL.md](../INSTALL.md) for the supported installation setups,
updates, chat configuration and removal. Preserve existing loader DLLs, INI and
history. Direct chat is opt-in; new package settings default to paced keyboard.

Reports should include version, DX11/DX12, loader/version, host/guest, hunt type,
reproduction steps and relevant log excerpts with personal data removed.

## Publication checklist

The tested archives are frozen; matching English texts, nine labeled images and
checksums are in dist/publication-0.6.11. Real-game screenshots can supplement the
diagnostic previews. No upload was performed. Review the page preview and publish
from the chosen account when ready. If publishing a repository, run its hosted CI
before claiming hosted verification. Expedition damage remains out of scope.
