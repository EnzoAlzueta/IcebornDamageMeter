# Validation status

## 0.6.15 compact width and official quest text candidate — 2026-11-18

All 54 local tests passed. New DX11/DX12 fixtures render the
Chinese quest from the report at width 260 and stress maximum counter values.
They check actual text metrics fit the numeric column without clipping/stacking.
Their diagnostic BMPs were visually inspected. Presentation tests check equal
live-panel height at 260/280/340/359/360/400/460/640 and keep detail-page geometry.
These are fixture checks; actual gameplay validation remains pending.

Read-only offline inspection of installed GMDs verified q01591_chS as 翔风之龙
with objective 讨伐钢龙. The old catalog explicitly substituted English for
Simplified Chinese. Official resources now supply 512 usable PC quest IDs with
names/objectives in fourteen game-text locales. The merged catalog has 562 IDs,
including 42 exact-English-title/objective aliases and eight unmatched legacy
IDs. Those legacy entries retain earlier fallback; unknown custom IDs are numbered.
Two Arabic gender tags use their first official variant; no gender is sampled.

The generic q30000 resources contain Unavailable rather than procedural target
information. ID 30000 therefore retains its translated Investigation label and
no guessed objective. Latin American Spanish uses spL text while the thirteen
UI languages still use the existing twelve translation columns. Localization
audit passed for 100 source strings. No game data is written and no archive
schema changes, per-frame file reads or translation/network dependency are added.

## 0.6.14 chat removal candidate — 2026-11-17

Removed both in-game chat transports, the menu send action, the chat-key setting
and send polling. The runtime no longer builds the chat input/memory writer.
Manual clipboard reports retain their existing format. Old ChatTransport and
ChatOpenKey INI entries are ignored; user settings/history need no migration.

All 50 local tests passed, including DX11/DX12 rendering, shorter menu/settings
layouts, clipboard report formatting, visibility recovery, settings persistence,
recent history and cumulative archive integrity. The removed native-chat-only
test accounts for the decrease from 51. The existing persistence fixture was
adjusted to the 17 settings slots. Localization auditing passed for 100 English
strings and twelve complete translation columns. Menu/settings diagnostic images
were visually inspected. No game was launched or chat submitted during validation.
A subsequent user playtest was reviewed on 2026-11-17. The latest 0.6.14 session
initialized in DX12 and logged one completed four-player hunt without recorded
errors. The ten-record recent file decoded successfully. The cumulative archive
contained eleven valid CRC-checked frames, no exact duplicate frames, and a tail
matching the ten recent records. Settings included ShowBadge=0 and AutoShow=1.

This does not establish visual hide/clipboard behavior, a restart cycle, game
FPS, crash-free exit or DX11 gameplay. Anonymized review evidence is retained in
build/validation-0614/playtest-review-20261117.json. The user then requested
preparation of the update for sharing without further code/logging changes.
Publication packages retain the tested DLLs; no game was launched by Codex.

## 0.6.13 visibility candidate — 2026-11-17

ShowBadge and AutoShow default to enabled for compatibility. Both are saved by
the background settings writer. Fully hidden panels have an empty input hotspot
and skip GPU initialization/composition; the normal poll still records hunts.
Manual visibility survives quest transitions when automatic opening is disabled.

All 51 local tests passed, including the new visibility fixtures, persistence
checks, quest transitions and the preceding archive tests. Localization auditing
passed for 110 English source strings and twelve complete translation columns.

New DX11/DX12 fixtures check hidden startup, real pixels absent while hidden,
recovery through synthetic visibility toggles and resize while hidden. DX11 also
checks that the compositor count does not advance during hidden frames. These
are diagnostic hosts, not proof that a physical F8 press works in every game
configuration. Real-game validation remains pending.

## 0.6.12 cumulative archive candidate — 2026-11-17

The cumulative archive is separate from the ten-record history and introduces no
per-frame archive operations or statistical analysis. It uses a fixed 32-record
producer queue and a storage-worker-owned file. Startup reads at most 28,547 bytes
including the header, and retains at most ten encoded records for deduplication.

All 49 local tests passed, including DX11/DX12 rendering regressions and Unicode record roundtrips,
bounded reopen, interrupted first/last-frame recovery, corruption preservation,
multi-hunt queue retention, opt-out and archive-error isolation. Startup import
also covers the archive being ahead of or behind the recent-history snapshot,
without cascading duplicate imports. Package checks accompany the prepared candidate.

Synthetic local benchmark: 1,000/10,000/100,000 hunts, four sample players,
Windows filesystem cache warm, 32 individually flushed measured appends per size.
Batch writes populate each fixture before timing. The median worker append took
approximately 1.65–1.79 ms, with observed p95 1.96–2.05 ms. Reopen took 8.05–11.67 ms;
all three read exactly 28,547 bytes and retained ten records. The standalone
benchmark process private bytes were 602,112 / 630,784 / 630,784; these are not
measurements of the full game or the entire meter's memory footprint.

Raw results: build/validation-0612/archive-benchmark-final.txt. This does not establish
zero FPS cost, absence of game-frame spikes, Intel laptop performance or safety
under every storage failure. Real-game playtesting remains pending. Archive
format, recovery limits and process-exit caveats are in [HUNT-ARCHIVE.md](HUNT-ARCHIVE.md).

## 0.6.11 field report — 2026-11-15

The user reports successful testing with their friend: everything works and no
further crashes occurred. Both installations already use 0.6.11. No test duration,
individual focus/Steam-overlay scenarios, driver version or new logs accompanied
this report, so it does not establish the original ERR12 cause or compatibility
across all systems. It supports preparing this version as a public beta.

The existing 46/46 automated tests passed before this report. The runtime, source
and minimal update archives are frozen without rebuilding or repacking, and the
publication kit now matches 0.6.11. The included package documentation retains
its earlier candidate assessment; the final report is recorded separately in
dist/publication-0.6.11/PLAYTEST-REPORT.md. No public upload was performed.
Real-game screenshots remain optional gallery additions; diagnostic previews
are labeled as sample data. Expedition/Guiding Lands damage remains unsupported.

## 0.6.11 investigation assessment before the field report

Friend report: Core i3-6006U, 8 GB RAM, integrated graphics, DX11 at 1024x768.
The CPU integrates HD Graphics 520 according to Intel; the active game adapter
still needs direct confirmation. The supplied appended log shows successful
initialization and many completed hunts but no device-removal diagnostic. User
later associated some failures with Task Manager or opening the Steam overlay.
No controlled no-meter baseline, driver version or reproduced causal crash exists.

The candidate reduces repeated DX11 UI raster/Flush work and suspends drawing
on focus loss/minimize. Game-frame composition and keyed-mutex synchronization
remain. Static-cache tests count rasterizations separately from composites and
exercise pixels/state/resize; focus is simulated in the hidden diagnostic host.
The real Steam overlay does not necessarily change the foreground window and
is not independently detected by this patch. At this stage no Intel laptop
validation had been reported; see the subsequent field report above.

Extra Present/adapter logs improve evidence; observing a device error after
Present does not identify its cause or repair a crashed game device. Publication
was held at this stage. The frozen 0.6.10 archives remain unchanged.

## 0.6.10 English standardization candidate

The 108 interface source strings and their lookup keys are now English. All
12 non-English translations are preserved; English remains the automatic fallback.
Identifiers, configuration keys, history formats and game language detection are
unchanged. Primary installation, release, compatibility, contribution and third-party
documentation is English; Spanish companions remain available. Package INI comments
and short guides are English. Earlier sections below retain their historical context.

The installed 0.6.9 is intentionally untouched. The new candidate requires a
real-game smoke test after installation; this language migration does not extend
the scope of earlier playtest reports.

## Current field report: 0.6.9

The user reported successful testing of the latest fixes: investigation labeling,
complete player names and native sharing after returning to town. This closes
those reported defects as user-validated on their installation. Earlier pending
statements below describe the state before that report.

The 45 local tests and package verification passed for the installed 0.6.9
candidate. This report does not independently establish quantitative accuracy,
all 13 language translations, four-player coverage or every loader/GPU combination.
Expedition damage is deferred by the user's explicit choice.

## 0.6.9 candidate

User confirmed native sharing during quest but reported city failure, shortened
Pharis name and generic Quest #30000. Log: native request during investigation
completed; city requests failed on the first submit step. A retained own buffer
is a plausible cause; no live memory snapshot confirmed it. The correction only
permits exact owned bytes in the same UI object when its flag is already clear.
Private-memory tests retain draft/edited/active/busy/page checks. Local tests cover
full names, Unicode limits, completed mission totals in town and procedural labels.
Actual city reception remains unvalidated. Previous icon correction is unchanged.

## 0.6.8 candidate

User reported Bow displayed as Light Bowgun and keyboard sharing was slow.
Inspection of the existing extracted atlas confirms ranged columns 11=LBG,
12=HBG,13=Bow, while native weapon IDs are 11=Bow,12=HBG,13=LBG.
Metadata and generated header now agree; regression assertions name each mapping.
Native chat behavior itself is unchanged from 0.6.7. Enabling it locally is a
test configuration change; it is not proof of online delivery. No test message
is sent automatically. Expedition measurement is deferred by the user.

## 0.6.7 field context and chat candidate

Local regression coverage now includes expedition entry with deliberately stale
counters, invalid samples, village return, active arena/event IDs, summaries and
visibility. This does not prove live expedition detection or arena/event behavior.
The point-in-time read-only sample obtained from the user's game showed Astera
(stage301, raw quest -1/state0), not an expedition; later the game was closed.
The log confirms installed 0.6.6 selected es through Steam's current GAME API.

Native chat is explicit opt-in, default keyboard unchanged. Its queue/memory tests
use private allocations only: bounds, draft/visible-entry rejection, vtable/page
guards, busy/consume/cooldown/timeout/focus states and no retry. No message was
sent to another player in these checks. UTF-8 limits are tested with long Latin,
CJK, Cyrillic and Arabic names across all 13 UI languages.

REA process capture and Rizin returned capability_unavailable on Windows; no
debugger or Frida injection was performed. Direct field observation used a local
ReadProcessMemory helper and is not REA Evidence. Native offsets are source-based
candidates from toolkit's 15.23 layout, not independently disassembly-verified.
Strict runtime structural checks may reject the direct route; that is preferable
to guessing or automatically duplicating an uncertain message through keyboard.

Pending: actual expedition damage needs another data source; online reception
and live direct-chat operation need user testing. Installed DLLs remain 0.6.6.

## 0.6.6 localization candidate

Prepared separately while the user tests installed 0.6.5. Automatic detection
uses documented exports from the already loaded Steam API; no Steam initialization,
injection offsets, network translation service or game settings writes.
Tests cover all 13 locales, placeholder preservation, quest fallback, localized
chat length, explicit overrides, late Steam readiness and graphics cache refresh.
Japanese and Arabic DX12 screenshots were visually inspected for glyph rendering.
The local test fixture is never included in runtime packages.

Real-game language detection and localized gameplay remain unvalidated. This
does not establish online chat delivery; the 0.6.5 online test is still pending.

The Windows build, statistics/state, background storage, bootstrap and local
DX11/DX12 tests run locally. The prepared GitHub workflow uses the non-graphics
preset; its first actual hosted run remains pending repository creation.

The current versions use manual file copying. Installation scripts and fixtures
have been removed. Runtime ZIPs are checked for the correct root/chain DLL, nativePC
plugin layout, documentation, manifest and DLL checksums, and absence of scripts.
These packaging checks do not certify another mod's compatibility.

Bootstrap tests use an isolated host and fixture plugin, not the actual game.
They exercise import-time startup, system XInput forwarding and ordinals,
concurrent initialization, a different working directory, duplicate loads,
missing plugin handling and rejection of another host.

Local graphics tests exercise early/late DX11/DX12 attachment, rendering, resizing,
weapon icons, zero-damage colors, compact view, menu, settings, history and record
details. Static composition is checked for cache reuse. Raw Input registration,
restoration and focus-loss cleanup are separate from real game/mouse behavior.

Seven DX11 cases use legacy/flip swapchains, Present/Present1, WARP and hardware,
game devices with/without BGRA support, pixels read back from actual GPU output,
pipeline-state preservation, two resizes and static-cache reuse. Readback is test
instrumentation only; the plugin shares a GPU texture without CPU readback.

An additional optional local integration used the real MHW mod loader 1.0.2.0
from the existing Seamless install, our chain DLL and a fixture plugin. It
confirmed chained startup, XInput pass-through and single plugin initialization.
No game was started and no third-party binary is shipped or needed for CI.

Older game testing confirmed display and increasing counters on the original
local installation. Those results do not certify gameplay on candidate 0.6.2.
Its current real-game evidence and remaining field tests are recorded below.

## 0.6.0 / 0.6.1 startup incidents (both withdrawn)

Windows Application Error recorded stack overflow 0xC00000FD in our 0.6.0 DLL,
offset 0x13586. The installed DLL matched the tested artifact's SHA-256. Ordinary
MSVC disassembly places that offset inside the Present callback's hook lookup.
Disabling our plugin and our chain DLL restored startup, confirmed by the user.

A copied DXGI table could save our Present detour as its own original, causing
unbounded recursion. A regression built against the archived 0.6.0 source
reproduced 0xC00000FD. The corrected source resolves only unique runtime originals
with compatible table identity and never saves a self callback as an original.
Four copied-table cases (DX11/DX12, early/late) now pass, for 23 local tests.
The exact external overlay responsible for cloning the live table is not proven.

Both runtime versions were withdrawn: 0.6.1 also failed in the real game. Its
Seamless trace showed repeated calls between Steam gameoverlayrenderer64.dll
and our Present callback. Candidate 0.6.2 uses per-target MinHook trampolines and
typed callbacks with TLS original-call context; 27 local tests passed before
the real-game checks below. Fixture coverage is not a stability certification.

## 0.6.2 real-game checks — 2026-11-11

Windows x64, game profile 421810, DX12 SDR 1280x720, Steam overlay present,
MHW mod loader 1.0.2.0 and Seamless were used. Installed plugin and bootstrap
SHA-256 matched the 0.6.2 manifest. Original loader, INI and disabled backups
were preserved; logs were copied before each launch.

- Combined Stracker + Seamless run (PID 9428): startup reached the title screen,
  live DX12 queue and swapchain were captured, and the D2D/DX12 renderer became
  ready. Four preview bars and F6 settings were visually verified. No immediate
  startup stack overflow recurred. The bootstrap logged Win32 error 126; Stracker
  loaded the plugin. This run alone does not validate the chain route. All direct
  system dependencies loaded in a separate probe; the error's cause is unresolved.
- Seamless-only run (PID 8628): Stracker's root dinput8.dll was temporarily
  renamed before startup and automatically restored with matching SHA-256 after
  module inspection. The process contained Steam overlay, Seamless, system
  DINPUT8 and our plugin, with no Stracker loader.dll or root DINPUT8. The
  bootstrap reported `plugin loaded (Win32 error 0)`. The loader's direct scan
  still ignored our foreign plugin, as expected. DX12 capture and renderer
  initialization succeeded, and four preview bars were visible in the real game.

Local evidence is retained in build/game-validation-062-20261111-182557
(not distributed): before/after logs, baseline hashes, module inventory and PNGs.
Preview values are fictitious, not a measurement of actual damage.

The Seamless-only run then failed during exit: immediately after reporting normal
Quit, its log recorded C0000005 at atidxx64.dll+0x3B7DF. The stack included Steam
overlay, D3D11 and our DLL+0x52161. MSVC disassembly plus a diagnostic relink map
identify this as the COM Release in the static destructor of bootstrapDevice11.
The relinked .text is byte-identical to the installed 0.6.2 .text (SHA-256
9d0ea5c8aa466138f699c7076249ef943402b23c27df69ed97e5759f602cfc34).
REA deep analysis was unavailable due to its JDK probe timeout; attribution uses
ordinary MSVC tools, not decompiler output. 0.6.2 runtime ZIPs are withdrawn.

Still pending: mission damage, multiplayer accuracy, real-game DX11,
clean installation without either external loader, other GPU/mod combinations,
frame-time/FPS measurements and longer play sessions.

## 0.6.3 candidate

Process-owned global graphics/DirectInput COM references now detach during static
destruction instead of entering drivers under the loader lock. Local ComPtr owners
and explicit Reset/reassignment retain normal release behavior. The DX11 surface
also remains process-owned. A regression checks normal releases before exit and
fails if a global COM Release occurs during static teardown. All 28 local tests
passed. Local DX12 SDR 1280x720 cycles then confirmed startup, four visible preview
bars, and normal Quit without a CRASH record, both with Stracker + Seamless
(PID 11920) and with Seamless alone (PID 10148). The latter bootstrap returned
Win32 error 0 and module inventory excluded Stracker. Its original DLL was restored
with matching hash before the test process exited. Steam overlay stayed enabled.
DX11 SDR 1280x720 with Stracker + Seamless (PID 10352) also passed startup,
swapchain capture, D2D/DX11 initialization, four visible preview bars and normal
Quit without a CRASH record. The temporary DirectX12Enable=Off setting was undone
after exit by restoring the exact saved graphics_option.ini bytes and verifying
its SHA-256; DX12 is restored. All three runs retained Steam overlay.

These are title-screen checks, not mission-damage or multiplayer validation.
Still pending on 0.6.3: mission damage, multiplayer accuracy, DX11 with Seamless
alone, clean install without external loaders, other hardware, FPS and long play.

### Subsequent multiplayer field report

The user shared the 0.6.3 Seamless candidate with a friend and subsequently
reported that their joint playtest worked perfectly as expected. This supplies
positive user-reported multiplayer gameplay evidence beyond the automated
title-screen checks above. No new logs, exact damage comparison, session duration,
quest details or friend's graphics mode were supplied; do not infer quantitative
accuracy or additional renderer coverage from this report. Long sessions, FPS,
other hardware and loader combinations remain open. Candidate status is retained.

## 0.6.4 candidate — quest identity, sizing and chat

The existing history format is unchanged. A compiled 521-entry MIT catalog maps
stored quest IDs to Spanish names/objectives, with English and explicit numeric
fallbacks. The local existing history contained IDs 408 and 251, both catalogued;
its file was inspected read-only and is not rewritten for this change.

Presentation tests cover names, unknown IDs, legacy-history roundtrip, 40% minimum
scale, 260 minimum width, row/button geometry for all panel pages across seven
widths, and group chat totals/shares/line lengths. Chat playback tests cover both
Insert/Enter bindings, a held mouse button, cooldown, non-overlap, key release on
focus loss, failure and Unicode boundary handling. Chat lines always carry a
prefix so a player name cannot start a chat command.

Eight additional DX11/DX12 GPU cases exercise 260-wide current/history/settings
and 260-wide/40% preview, read actual output pixels, resize twice and preserve the
game pipeline state. These and the earlier regressions passed (37 tests total).
Snapshots were visually inspected for the history names and narrow layout.

Chat uses timed keyboard events plus WM_CHAR in the focused game window; it does
not call an unverified internal function or use the clipboard. It is manual only,
blocks preview/no-data sharing, retains a single snapshot, limits line length,
and cancels pending actions on focus loss/Escape. The current game chat channel
and supported chat-open key must be set correctly. Passing playback tests does
not certify actual game chat delivery; multiplayer field validation of 0.6.4
remains required. The 0.6.3 user's gameplay report does not certify this update.

The 0.6.4 candidate was then installed with both DLLs backed up from 0.6.3.
A real-game DX12 + Stracker + Seamless run (PID 9328) logged bootstrap error 0,
matching game hash, live queue/swapchain capture and renderer ready. It stayed
open for about five minutes and Quit completed normally without a CRASH record.
The desktop focus could not be transferred to the game, so no key was sent by
the history inspection helper and real-history visual validation remains open.
The narrow/history screenshots inspected above are GPU fixture output. No actual
chat messages were sent during validation. INI, history and both external loaders
were preserved; final SHA-256 checks compare them with the pre-install baseline.

## 0.6.5 candidate — field report and corrections

The user reported that 0.6.4 sometimes rejected visible damage while a mission
was in progress, and that its chat attempt produced no message after the mission.
The real log contains four accepted requests for quest251/total1079 with completed
input sequences, not delivery acknowledgements. The user confirmed that Insert
and Enter did not open the normal game chat even with the meter menu closed, and
that the test was solo. Seamless logged no online session during those attempts.
This establishes unavailable chat in that test; it does not certify delivery in
an online session or prove every possible cause of the chat UI being unavailable.

0.6.5 removes the latest-sample availability gate from sharing cached, validated
mission totals. Tests exercise Live and Summary with an invalid latest sample and
reject a new zero-damage mission and preview. Busy attempts get separate feedback.
The input path now uses scan-code key events (including explicit extended Insert)
and Unicode input events; compact UI exposes cancellation/failure and explicitly
unconfirmed delivery. A separate user action copies the actual summary to clipboard.
The automatic path does not touch clipboard contents.

Original weapon icons were extracted from the installed chunkG0.bin texture
ui/common/tex/cmn_item00_ID.tex, whose SHA-256 is
7e8bd3081253a0b84f7e86553cba1e2e3159c74c4a0cb9ab51f72432ca35da3d.
The bottom equipment row contains14 cells of64x64; assets/game_weapon_icons.json
records the native-type remapping. The cropped PNG is embedded, decoded once by
system WIC, and cached in a GPU bitmap. Gallery output with all14 icons and labels
was inspected visually. Graphics regressions require the atlas to load, and cover
DX11/DX12, minimum size, cache reuse and resize.

Extraction/decompression used local research helpers plus upstream format references
and powzix/ooz built under build/research-065. No REA tools were registered in this
turn; findings are ordinary shell/format-inspection evidence, not REA Evidence.
Neither these helpers, the decoder nor the original full game atlas is packaged.
The crop retains CAPCOM artwork attribution separate from the project's MIT code.

Real 0.6.5 chat delivery must still be tested in an online session where normal
chat already opens manually. Do not equate successful SendInput with a delivered
message. No chat messages were sent to other people during development.
Final local suite:37/37 passed, including scan-code Insert, completed cached
mission eligibility, and all GPU cases with the original icon atlas loaded.
A subsequent real DX12 + Seamless + Stracker run (PID3184) logged bootstrap error0,
original game weapon icons ready, and renderer ready. The preview and real F7
history with three saved hunts were visually inspected in the game. Normal Quit
completed without CRASH. No chat was sent; the test stayed on the title screen.
INI, history and both external loaders matched the pre-install baseline afterward.
Evidence: build/game-validation-065-20261111-212237 (local, not distributed).
