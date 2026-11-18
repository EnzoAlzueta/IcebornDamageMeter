ICEBORN DAMAGE METER - INSTALLATION

Close the game. Steam > Manage > Browse local files opens the EXE folder.
For a new meter installation, merge nativePC into the game folder.
Without a loader, also copy xinput9_1_0.dll. With Stracker, copy ONLY nativePC.
Never overwrite another mod's root or chain XInput DLL.

Use DX11 or DX12 with SDR; the API is detected automatically.
F8 show/hide; Ctrl+F8 preview; F6 settings; Ctrl+F6 menu; F7 history; Ctrl+F9 move.
If the meter already works, replace only nativePC/plugins/iceborne_damage_meter.dll
from this ZIP. Keep your loader, INI and saved hunts. Remove our plugin to uninstall.
Never delete nativePC or another mod's loader.

English guide: IceborneDamageMeter/INSTALL.md
Spanish guide: IceborneDamageMeter/INSTALL.es.md
The menu can copy a damage summary to the clipboard. In-game chat sending is removed.
Language=auto follows the game's Steam language. Expeditions have no damage data.

Logs: nativePC/plugins/iceborne_damage_meter.log and the bootstrap log beside EXE.
No installer is required. Source and runtime packages are different artifacts.

ArchiveHunts=1 under [Overlay] saves cumulative binary hunt summaries in the
background; the visible history still keeps ten hunts. Set 0 and restart to opt out.
Keep nativePC/plugins/iceborne_damage_meter_hunts.bin when updating.

Downloads and reports: https://www.nexusmods.com/monsterhunterworld/mods/8969
