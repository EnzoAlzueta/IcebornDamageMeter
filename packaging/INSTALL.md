# Iceborn Damage Meter — installation and use

[Español](INSTALL.es.md) · [Credits and licenses](THIRD_PARTY.md)

## Requirements

Monster Hunter World: Iceborne, Windows 10/11 x64, DirectX 11 or 12 with SDR,
and the supported executable for profile 421810. The included manifest records
the executable hash. No separate overlay application or development tools are
required. The standard package includes our bootstrap; Stracker is not required
for a clean installation.

## Install

1. Close the game. In Steam, choose Manage > Browse local files.
2. Copy the ZIP's `nativePC` folder beside `MonsterHunterWorld.exe`.
3. For a clean installation, also copy the standard ZIP's `xinput9_1_0.dll`.
   If Stracker is already installed, copy only `nativePC` and keep its loader.

Never overwrite another mod's root or chain DLL. If your setup already has an
XInput proxy from another mod, check its loading instructions first. The separate
chain package is for a compatible existing chain loader: copy its `nativePC` and
`xinput9_1_0.chain.dll`, keeping the existing root DLL. Do not use the chain package
as a clean installation or replace someone else's chain DLL.

The `IceborneDamageMeter` folder contains instructions, licenses and a manifest,
not an extra application. The source ZIP is for developers.

## Update or remove

Close the game. Keep your existing `iceborne_damage_meter.ini` and
`iceborne_damage_meter_history.dat` under `nativePC/plugins`; do not replace
them with defaults from a new package. Replace only our plugin and, when a
release requires it, our matching bootstrap. Keep other mods' loaders intact.

To remove the meter, delete only `nativePC/plugins/iceborne_damage_meter.dll`.
Keep settings/history if wanted. Remove our bootstrap only after verifying it
belongs to this package and is not needed by your setup. Never delete `nativePC`.

## Controls and layout

The panel appears automatically during a mission. Default controls:

| Key | Action |
|---|---|
| F8 | Show/hide |
| Ctrl+F8 | Preview with sample data |
| F6 | Settings |
| Ctrl+F6 | Menu and manual sharing |
| F7 | Recent hunt history |
| Ctrl+F9 | Unlock/lock dragging |

Existing INI key bindings are preserved. Settings include scale 40–175%, width
260–640, opacity, rows and columns. Settings and the ten-hunt history are saved.
Observed DPS includes time present, including travel and pauses; it is not active
attack time or a normalized skill score. Multi-target quests show mission totals.

F6 also offers **Show DM button** and **Show on quest start**. Turn off the first
to remove the small DM shortcut while the panel is hidden. Turn off the second
to start hidden and use the show/hide key manually across quests. Both settings
are saved. With both off, F8 (or your configured show/hide key) still opens the
panel; F6, F7 and Ctrl+F6 also open their respective pages. Recording continues
while hidden. No panel/badge GPU drawing occurs when both are hidden.

INI equivalents under `[Overlay]`: `ShowBadge=0` and `AutoShow=0`.

## Long-term hunt archive

`ArchiveHunts=1` under `[Overlay]` enables
`nativePC/plugins/iceborne_damage_meter_hunts.bin`. It appends hunt summaries in
the background for a future external analyzer; the meter still displays ten
recent hunts and does not analyze or load the full archive. Existing recent
records newer than the latest common archive entry are imported on startup;
if no common entry exists, the available recent snapshot is imported.

Set `ArchiveHunts=0` with the game closed to disable archiving. Preserve the
archive when updating. To remove it, close the game and delete only that file;
the ten-entry history is separate. Disk/file errors leave the recent history
active and are reported in the plugin log. A sudden exit can lose queued data.

## Language

`Language=auto` under `[Overlay]` in the INI follows the game's Steam language.
Restart the game after changing it. Optional overrides: `en`, `es`, `pt-BR`,
`fr`, `de`, `it`, `ja`, `ko`, `zh-CN`, `zh-TW`, `ru`, `pl`, `ar`.
Missing quest translations fall back to English. Player names are preserved.

## Share damage

Open the menu and choose the current-damage chat action. Recorded mission totals
remain available after returning to town until a new mission resets the meter.
Preview data and empty missions cannot be shared. Copy summary is the explicit
clipboard alternative.

New settings use `ChatTransport=keyboard` under `[Overlay]`, with paced input.
`ChatOpenKey=45` means Insert; `13` means Enter. Match the game's chat binding.
For experimental direct submission without typing, close the game and set
`ChatTransport=native` in that section. Preserve the rest of your INI. Direct
mode protects other drafts and does not automatically retry or switch to keyboard.

Use an online session and the intended chat channel. Wait at least 15 seconds
between requests. Long groups may need several messages. Confirm reception with
another player; local request completion does not confirm delivery. Sharing
occurs only when you request it.

## Limitations and troubleshooting

Expedition/Guiding Lands damage is unsupported; those contexts show no data.
Generic investigation ID 30000 is labeled Investigation, without an invented
target, and excluded from personal DPS comparison. Unknown quests remain numbered.
HDR and device-loss recovery are unsupported. Not every GPU, language, party
size or loader combination has been independently validated.

If no panel appears, try Ctrl+F8. Check
`nativePC/plugins/iceborne_damage_meter.log` and the bootstrap log beside the game
EXE. For a graphics crash, include GPU/driver, API, mod version and reproduction
steps. Remove personal data from logs and screenshots.

Downloads and reports: https://www.nexusmods.com/monsterhunterworld/mods/8969
