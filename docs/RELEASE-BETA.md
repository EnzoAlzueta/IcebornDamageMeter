# Iceborn Damage Meter 0.6.14 — beta update

Changes since 0.6.11-r2:

- Hide the small DM button with a saved setting.
- Choose whether the panel opens at quest start. Manual controls remain available.
- Continue recording while hidden; skip panel/badge drawing when fully hidden.
- Optionally save cumulative hunt summaries for future external analysis.
  The in-game history still shows the ten most recent hunts.
- Remove in-game damage chat sending. Copy damage summary remains available.

## Update

If the meter already works, close the game and use the update-plugin-only ZIP.
Copy nativePC into the game folder and replace iceborne_damage_meter.dll.
The update contains no INI, loader, saved hunts or personal data. Your existing
loader, settings and history stay in place. Old chat settings are ignored.

New users need the full windows-x64 ZIP and its installation guide. A clean
installation uses nativePC plus the included XInput bootstrap. Existing loaders
must follow their compatible loading route; never overwrite another mod's loader.
The source ZIP is for developers, not game installation.

## Archive and controls

ArchiveHunts=1 under [Overlay] is the default even if absent from an existing INI.
It appends summaries to nativePC/plugins/iceborne_damage_meter_hunts.bin on a
background worker. Set ArchiveHunts=0 and restart to stop future archiving.
Keep that file and iceborne_damage_meter_history.dat when updating. The new file
does not recover hunts already discarded before archiving was introduced.
No archive analysis or weapon ranking runs in the game.

F6 offers Show DM button and Show on quest start; both default to enabled.
F8 show/hide, Ctrl+F8 preview, Ctrl+F6 menu, F7 history and Ctrl+F9 dragging are
the defaults. Existing customized key bindings are preserved. Copy damage summary
is in the menu and writes only to the clipboard; nothing is sent to game chat.

## Validation and limits

Fifty local tests passed, including DX11/DX12 diagnostic rendering and storage.
A subsequent user DX12 session archived one completed four-player hunt without
recorded errors. Recent/cumulative files passed independent format/CRC checks.
This is limited field coverage, not universal compatibility, proof of every
visual control, a clean exit, DX11 gameplay or a measured game FPS improvement.

Expedition/Guiding Lands damage, HDR and device-loss recovery remain unsupported.
Logging is unchanged and currently has no size cap or rotation.

Nexus: https://www.nexusmods.com/monsterhunterworld/mods/8969
Source: https://github.com/EnzoAlzueta/IcebornDamageMeter

Prepared locally; publication is a separate action.
