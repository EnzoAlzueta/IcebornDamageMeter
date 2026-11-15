# Third-party notices and technical references

[Español](THIRD_PARTY.es.md). Original project code uses the MIT license.
References below describe their own licenses; they do not relicense external
code or game artwork as project code.

## MinHook

MinHook v1.3.4 is statically linked under its BSD 2-Clause license. Source and
notice: third_party/minhook; runtime notice: licenses/MinHook-LICENSE.txt.
https://github.com/TsudaKageyu/minhook/tree/v1.3.4

## HunterPie layout references

Memory fields for party counters, players, quests and stage IDs were consulted
in HunterPie (Apache-2.0). This does not include its bypass or patching routines.

- https://github.com/HunterPie/HunterPie/blob/main/HunterPie/Address/MonsterHunterWorld.421810.map
- https://github.com/HunterPie/HunterPie/blob/main/HunterPie.Integrations/Datasources/MonsterHunterWorld/Entity/Player/MHWPlayer.cs
- https://github.com/HunterPie/HunterPie/blob/main/HunterPie.Integrations/Datasources/MonsterHunterWorld/Definitions/MHWPartyMemberStructure.cs
- https://github.com/HunterPie/HunterPie/blob/main/HunterPie.Integrations/Datasources/MonsterHunterWorld/Definitions/MHWQuestStructure.cs
- https://github.com/HunterPie/HunterPie/blob/main/HunterPie.Integrations/Datasources/MonsterHunterWorld/Entity/Enums/QuestState.cs
- https://github.com/HunterPie/HunterPie/blob/main/HunterPie.Core/Game/Enums/Stage.cs

## Native chat field reference

The bounded native chat implementation uses the 15.23 layout published in
mhw-toolkit, Copyright 2024 Eigeen, Apache-2.0, as a technical reference. The
toolkit library is not linked or bundled. Its license is retained in
third_party/mhw-toolkit/LICENSE.txt and runtime notices.

- https://github.com/eigeen/mhw-toolkit/blob/main/src/game_export.rs
- https://github.com/eigeen/mhw-toolkit/blob/main/src/game_util.rs

Expedition limitations and alternative approaches were researched in SmartHunter;
no expedition synchronization or external server from that project is included.
https://github.com/gabrielefilipp/SmartHunter

## Quest catalog

Names/objectives derive from gatheringhallstudios/MHWorldData, MIT,
Copyright (c) 2018 Carlos Fernandez, pinned revision
be7362213d7d1e30b794e3b58d3f87712035658d.
https://github.com/gatheringhallstudios/MHWorldData

third_party/mhworlddata contains the reduced ID/name/objective CSV, README and
license. scripts/generate-quest-catalog.py generates the offline header.
Runtime packages include MHWorldData-LICENSE.txt. Missing translations use English;
upstream Chinese titles are Traditional, not Simplified.

## Game artwork

The fourteen weapon icons are extracted from the local game's
ui/common/tex/cmn_item00_ID.tex in chunkG0.bin. Copyright CAPCOM. They are separate
from the MIT code license. Asset metadata records the texture digest, crop and
native-type mapping. See assets/Game-artwork-NOTICE.txt. Public redistribution
remains a release-review item; an attribution notice is not a distribution grant.

The PNG is embedded by scripts/embed-weapon-icons.py and decoded once through
Windows WIC into a cached GPU texture. No full atlas, ooz, MHWNoChunk, extraction
tool or third-party loader is included in runtime packages.

Weapon colors are contrast-adjusted references to MHW guild-card hues, not
official RGB values. Earlier versions used original code-drawn silhouettes.
https://wikiwiki.jp/nenaiko/ゲーム用語/アイコン#武器アイコンの色

## Windows APIs and loaders

The original MIT bootstrap forwards XInput calls to the system DLL and loads
only this meter. Stracker loads plugins directly; Seamless's loader provides
XInput chaining. Neither loader is bundled or covered by this project's license.

Microsoft API references:
- https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-versions
- https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryexw
- https://learn.microsoft.com/en-us/windows/win32/direct3d12/d2d-using-d3d11on12
- https://learn.microsoft.com/en-us/windows/win32/direct2d/devices-and-device-contexts
- https://learn.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-swapdevicecontextstate
- https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgikeyedmutex-acquiresync
- https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerrawinputdevices
- https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getregisteredrawinputdevices
- https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawinputdevice

DX11 uses a private BGRA device, shared GPU texture and context-state restoration;
DX12 uses D3D11On12. Temporary edit-mode mouse registration is restored on exit.
