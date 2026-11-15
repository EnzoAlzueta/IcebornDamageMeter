# Installation and usage

## Prepared public beta 0.6.11

The user reported successful testing with a friend and no further crashes on
2026-11-15. Both installations already use this version. The tested archives are
unchanged; preparing the publication kit requires no further installation.

DX11 can reuse static panel pixels, reducing repeated UI raster/Flush work.
Drawing pauses when the game loses foreground focus or is minimized; stats
continue to be polled. No configuration change is required. Opening the Steam
overlay may retain game focus, so it is not independently excluded by this guard.
The original ERR12 cause is not confirmed; the beta has limited field coverage.

If a graphics crash recurs, keep a backup and compare the same DX11 activities
with the meter disabled and enabled. Save logs, note focus/Steam-overlay activity
and confirm the driver version. Logs include adapter identity and any returned
Present/device-removal HRESULT. See docs/VALIDATION.md for the latest field report.

[Español](INSTALL.es.md) · [Compatibility](docs/COMPATIBILITY.md)

## Requirements

Monster Hunter World: Iceborne, Windows 10/11 x64, DirectX 11 or 12 with SDR,
and the supported executable for memory profile 421810. An unsupported executable
disables damage reading. No separate overlay application is required.

## Choose a package

Close the game. In Steam, choose Manage > Browse local files to find the folder
containing `MonsterHunterWorld.exe`.

| Existing setup | Files to copy beside the EXE |
|---|---|
| No mod loader | Normal ZIP: `nativePC` and `xinput9_1_0.dll` |
| Stracker's Loader | Normal ZIP: only `nativePC` |
| Seamless | Seamless ZIP: `nativePC` and `xinput9_1_0.chain.dll` |

Keep Seamless's original `xinput9_1_0.dll`. Never overwrite a root or chain DLL
owned by another mod. An unknown existing XInput proxy needs its own compatibility
instructions. The minimal friend ZIP is an update for an existing Seamless setup;
it contains no INI or saved history.

Merge folders. When updating, keep your existing INI and history; replace only
this mod's plugin and bootstrap with the matching package versions. There is no
installer or installation command. Do not copy the source ZIP into the game.

## Controls

| Default key | Action |
|---|---|
| F8 | Show/hide |
| Ctrl+F8 | Synthetic preview; never saved as real damage |
| F7 | Hunt history |
| F6 | Settings |
| Ctrl+F6 | Menu |
| Ctrl+F9 | Unlock/lock dragging |

Existing bindings are preserved. Settings include scale (40–175%), width
(260–640), opacity, visible rows, columns and key bindings. Settings save
automatically. The panel appears automatically during a mission.

## Language

`Language=auto` in `[Overlay]` follows MHW's language configured in Steam, not
the Steam client's language. Fallbacks are the MHW manifest, Windows locale and
English. Steam detection retries during the first minute. The mod does not change
game or Steam settings. Restart the game after changing its language.

Optional overrides: `en`, `es`, `pt-BR`, `fr`, `de`, `it`, `ja`, `ko`, `zh-CN`,
`zh-TW`, `ru`, `pl`, `ar`. A missing setting also means automatic detection.
Missing quest translations use English; the upstream quest catalog has no
Simplified Chinese titles. Player names and saved data are preserved.

## Share damage

Open the menu and select the current-damage chat action. Sharing uses actual
recorded totals, including the completed mission after returning to town, until
a new mission resets the meter. Preview and empty missions cannot be shared.
Generic investigation ID 30000 is labeled Investigation; no target is guessed.

Under `[Overlay]`, `ChatTransport=keyboard` uses paced normal input; set
`ChatOpenKey=45` for Insert or `13` for Enter to match the game controls.
`ChatTransport=native` selects experimental direct submission without opening
chat or typing. It was user-tested on the local installation, including town
sharing; other configurations still need validation. Existing nonempty drafts,
active entry and uncertain sends remain protected. No automatic keyboard fallback.

Use an online session and the intended game chat channel. Wait at least 15 seconds
between requests and confirm reception with another player. Escape/focus loss
cancels pending work; an already submitted request cannot be recalled. Names are
preserved when each entry fits. Long groups can require several bounded messages.
An input completion or consumed request flag does not certify network delivery.

Copy damage summary explicitly copies the fuller report to the clipboard. Neither
chat transport modifies the clipboard. Sharing only occurs on a user action.

## Limitations and troubleshooting

Expedition/Guiding Lands damage is not supported: the panel shows no data rather
than reusing a previous mission. Investigations sharing ID 30000 are excluded
from personal DPS comparison because they may have different targets/conditions.
Unknown quest IDs remain numbered. DPS is observed participation damage, not
active attack time. See [statistics](docs/METRICS.md) and [validation](docs/VALIDATION.md).

If the panel is missing, try Ctrl+F8 and inspect
`nativePC/plugins/iceborne_damage_meter.log`. The bootstrap log is beside the
game EXE. With Seamless, also inspect `nativePC/plugins/mhw-mod-loader.log`;
direct loading may be ignored while the chain loads the plugin successfully.

## Remove

Close the game and remove only `nativePC/plugins/iceborne_damage_meter.dll`.
Keep INI/history if you want them for a later installation. Optionally remove
our root/chain DLL only after verifying it still belongs to this package.
Never delete the whole nativePC folder or another mod's loader.
