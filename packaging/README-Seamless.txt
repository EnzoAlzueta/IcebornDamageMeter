ICEBORN DAMAGE METER - INSTALLATION

Close the game. Steam > Manage > Browse local files opens the EXE folder.
For an existing Seamless installation, use this Seamless package:
copy nativePC and xinput9_1_0.chain.dll beside MonsterHunterWorld.exe.
Keep Seamless's original xinput9_1_0.dll. Do not overwrite another mod's chain.
Copying nativePC alone may not load this plugin through Seamless.

Use DX11 or DX12 with SDR; the API is detected automatically.
F8 show/hide; Ctrl+F8 preview; F6 settings; Ctrl+F6 menu; F7 history; Ctrl+F9 move.
Preserve your INI and history when updating. Remove only our plugin to uninstall.
Never delete nativePC or another mod's loader.

English guide: IceborneDamageMeter/INSTALL.md
Spanish guide: IceborneDamageMeter/INSTALL.es.md
The menu can copy a damage summary to the clipboard. In-game chat sending is removed.
Language=auto follows the game's Steam language. Expeditions have no damage data.

Logs: nativePC/plugins/iceborne_damage_meter.log and the bootstrap log beside EXE.
With Seamless, also inspect nativePC/plugins/mhw-mod-loader.log.
No installer is required. Source and runtime packages are different artifacts.

ArchiveHunts=1 under [Overlay] saves cumulative binary hunt summaries in the
background; the visible history still keeps ten hunts. Set 0 and restart to opt out.
Keep nativePC/plugins/iceborne_damage_meter_hunts.bin when updating.

Downloads and reports: https://www.nexusmods.com/monsterhunterworld/mods/8969
