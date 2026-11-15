# Compatibility

| Component | Scope |
|---|---|
| OS | Windows 10/11 x64 |
| Game | MHW: Iceborne, memory profile 421810 and exact supported executable hash |
| Graphics | DX11 or DX12, automatically selected, 8-bit SDR |
| Loading | Included bootstrap; Stracker directly; Seamless through XInput chain |
| Unsupported executable | Damage reading disabled |
| Expeditions / Guiding Lands | Damage measurement unsupported |
| HDR / device-loss recovery | Outside current renderer support |

The package manifest records the supported hash. Loading, graphics and memory
compatibility are distinct: a successfully loaded DLL does not prove correct
rendering or supported data offsets. Never disable the hash guard to claim a port.

Use the [installation matrix](../INSTALL.md) and preserve existing loaders. Seamless
can ignore foreign plugins during direct loading; its chain starts our bootstrap.
Another mod's root or chain DLL must not be overwritten. Existing F10 bindings may
conflict with Seamless; new configurations use F6 for settings.

Local tests cover DX11/DX12 rendering, resize, legacy/flip swapchains, BGRA and
non-BGRA game devices, state preservation and cache reuse. Earlier real-game
startup/render/exit checks covered both APIs. The latest 0.6.9 gameplay fixes were
user-tested locally, including native sharing in town. These observations do not
certify every GPU, loader combination, language or four-player session.

0.6.10 standardizes source/documentation language; its real-game smoke test is
pending. Versions 0.6.0, 0.6.1 and 0.6.2 were withdrawn after startup/shutdown
failures. Do not redistribute or restore those versions. See [validation](VALIDATION.md).

Damage in normal quests, arena and events follows quest counters. Procedural
investigation ID 30000 is labeled Investigation and excluded from comparisons.
Expedition contexts display no data. Unknown/custom quest IDs stay numbered.

For reports, include mod version, API, loader/version, host/guest, quest type and
reproduction steps. Inspect the meter log and separate bootstrap log; with
Seamless also inspect mhw-mod-loader.log. Remove personal data before sharing logs.
