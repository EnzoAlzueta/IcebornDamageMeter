# Changelog

## 0.6.15 — development candidate

- Keep the compact damage meter on one line per hunter below width 360, down to
  the existing minimum 260. Narrow live rows allocate more space to numbers and
  fit long values when layouts rebuild; detailed pages keep their stacked mode.
- Prefer official game names/objectives for 512 installed PC quest IDs, including
  Simplified Chinese. Expand the catalog with verified game IDs and exact-text
  aliases for legacy database IDs. No runtime extraction or translation service.
- Select Latin American Spanish quest text separately while reusing Spanish UI.
- Keep a translated Investigation label for generic ID 30000: its text resource
  contains Unavailable, so this change does not invent dynamic targets.
- Preserve history/archive formats, visibility behavior, removed chat and logging.
- Add narrow Chinese/max-counter DX11/DX12 regressions. Local validation below.

## 0.6.14 — beta update

This update also includes the unpublished 0.6.12/0.6.13 work:

- Add saved options to hide the DM shortcut and disable automatic quest opening.
  Recording continues while hidden; fully hidden panels skip GPU drawing.
- Add an optional cumulative binary hunt archive on the storage worker, separate
  from the ten-hunt UI. No archive analysis runs in-game.

- Remove in-game damage chat sending, both keyboard and native transports,
  along with its menu action, chat-key setting and background send polling.
- Keep manual Copy damage summary, recent history and cumulative hunt storage.
- Ignore old ChatTransport/ChatOpenKey INI entries without rewriting user files.
- Update all UI translations and current installation guides.
- Local automated validation and real-game status are recorded in docs/VALIDATION.md.
- Provide a plugin-only update package that preserves existing loaders and data.
- Keep the same tested DLLs. Logging behavior remains unchanged, with no size cap.


## 0.6.13 (candidate)

- Add saved settings to hide the small DM shortcut and disable automatic panel
  opening at quest start. Defaults preserve the previous behavior.
- Keep the show/hide, settings, menu and history keys available with the DM
  shortcut hidden. Manual visibility remains under user control across quests
  when automatic opening is disabled; a new session starts hidden in that mode.
- Skip GPU initialization/drawing and clear the input hotspot when no panel or
  shortcut is visible. Damage/history/archive recording continues normally.
- Localize the two settings in all thirteen interface languages; add DX11/DX12
  pixel checks for hidden startup, resize while hidden and visibility recovery.
- Include the cumulative archive introduced in the preceding 0.6.12 candidate.

## 0.6.12 (candidate)

- Add an optional cumulative binary hunt archive for future external analysis;
  keep the existing ten-hunt history, UI, sampling and statistics unchanged.
- Append versioned, checksummed hunt summaries on the storage worker through a
  fixed queue. Never scan the full archive or compute aggregates in-game.
- Check only the bounded file tail at startup, import missing recent records,
  recover recognized interrupted trailing writes and preserve corrupt files.
- Archive errors are isolated from recent-history and settings persistence.
  Set ArchiveHunts=0 under Overlay and restart to opt out.
- Document the binary format and add storage/recovery tests and a separate
  synthetic archive-size benchmark. Real-game performance validation is pending.

## 0.6.11 package revision 2

- Remove development, validation, release-planning and localization-maintenance
  documents from installable ZIPs; retain them in the developer source package.
- Supply current English/Spanish installation and usage guides without historical
  candidate instructions, plus the required licenses, credits and manifest.
- Use an explicit runtime document list and fresh staging directories so removed
  files cannot leak back into later packages.
- Create the output directory when building packages from a fresh source checkout.
- Preserve the exact tested 0.6.11 plugin, bootstrap and default INI bytes.
  This is a packaging revision, not a gameplay or graphics change.

## 0.6.11 (prepared public beta)

- On 2026-11-15, the user reported successful testing with a friend and no further
  crashes. Freeze the tested archives and prepare matching publication materials
  without rebuilding. No public upload has been performed.

- Reuse static DX11 panel pixels between content/placement/edit changes instead
  of repainting the shared UI texture and flushing its producer every frame.
  Keep composition per game frame; do not throttle the panel into flickering.
- In normal game builds, skip drawing and lazy GPU initialization while the
  game window is unfocused or minimized. Continue polling gameplay state.
- Log the active DX11 adapter and failed game Present/device removal HRESULTs;
  preserve the original Present return value and disable further meter rendering.
- Add graphics checks for focus loss/resume and real pixel reuse across resizes.
- This is not a confirmed fix for the reported ERR12 on integrated Intel graphics.
  It does not detect Steam overlay visibility independently of window focus.

## 0.6.10 (candidate)

- Standardize English source strings, documentation, package guides and INI
  comments while preserving all 12 other interface translations.
- Add primary INSTALL.md, localization guidance, release draft and release plan;
  preserve Spanish companions and update third-party notices and compatibility.
- Replace outdated README status with the prepared candidate's actual scope.
- Preserve INI keys, history format and automatic game-language selection.

## 0.6.9 (candidate)

- Label observed generic investigation ID 30000 as Investigation in all locales,
  including existing history. Do not guess its targets or classify unrelated IDs.
- Exclude that shared procedural ID from personal DPS comparisons: matching IDs
  do not imply matching monsters or investigation conditions.
- Preserve player names in compact chat; pack complete entries into bounded
  messages instead of always abbreviating for four maximum-damage players.
- Allow native chat to replace its exact previous submitted buffer in the same
  UI object after the send flag clears. Continue rejecting foreign/edited drafts,
  active entry, pending flags and changed UI objects. No automatic retry.
- Live city sharing and investigation labels still require field validation.

## 0.6.8 (candidate)

- Correct the Bow/Light Bowgun atlas mapping: native Bow 11 uses texture column
  13, Heavy Bowgun 12 stays on 12, and Light Bowgun 13 uses column 11.
- Generate the mapping from the checked-in asset metadata instead of a second
  hard-coded list. Add explicit ranged-weapon regression assertions.
- Native chat from 0.6.7 is available through ChatTransport=native for sending
  without simulated typing. Still experimental and unvalidated with a recipient;
  packages retain keyboard as default. No new network transport was added.
- Expedition damage remains outside scope by user choice.

## 0.6.7 (candidate, not installed)

- Detect no-quest open-map/Guiding Lands contexts through the verified profile's
  stage field. Clear stale mission rows and show Expedition / No data instead.
  Native quest-award counters do not provide reliable expedition team damage;
  actual expedition damage measurement remains unsupported.
- Keep the preceding hunt in history before clearing the display. Interrupted
  live hunts are marked partial/withdrawn, never successful comparisons.
- Add regression cases for Arena Quest 01, Special Arena and Greatest Jagras
  event IDs, including totals, summaries and visibility. These are local tests,
  not live-game arena/event certification.
- Fit a normal four-player party in one compact chat message with shortened
  names and exact damage values. Full names/percentages/mission remain in Copy
  summary. Bound messages to 80 UTF-16 units and 127 UTF-8 bytes.
- Pace keyboard input per Unicode character (keep surrogate pairs together),
  wait before submission and between messages, and retain focus/Escape cancellation.
- Add explicitly opt-in ChatTransport=native for the verified executable only:
  stage UTF-8 into the game UI chat buffer and set its request flag, without
  opening chat, keyboard input or channel changes. Reject active text entry,
  existing drafts, invalid vtables/pages and busy flags; never retry a submitted
  message or fall back to keyboard on uncertainty. Default remains keyboard.
- Direct chat and expedition detection still need live-game tests. A consumed
  request flag, like successful keyboard input, does not confirm delivery.

## 0.6.6 (candidate, prepared; not installed)

- Automatically follow the game's Steam language, with manifest, Windows locale
  and English fallbacks. Optional INI override; no game settings are changed.
- Translate 103 UI strings, weapon labels, notices, exports and shared summaries
  into Spanish, English, Portuguese, French, German, Italian, Japanese, Korean,
  Simplified/Traditional Chinese, Russian, Polish and Arabic.
- Resolve existing history quest IDs using multilingual upstream names and
  objectives. Missing translations use English, including Simplified Chinese
  quest titles. Preserve player names and stored history.
- DirectWrite locale and Arabic reading direction; retry late Steam startup and
  invalidate cached presentation when the language changes.
- Real-game automatic detection and localized playtesting remain pending.
  Online chat delivery on the installed 0.6.5 remains a separate pending check.

## 0.6.5 (candidate)

- Do not reject the visible mission totals just because the latest memory sample
  is temporarily unavailable. Completed mission totals remain shareable.
- Report an attempt already in progress separately from missing damage; display
  completion/cancellation/failure feedback in the compact panel too. Completing
  an input sequence is explicitly not a delivery acknowledgement.
- Use hardware scan codes for Insert/Enter/Escape and Unicode input events for
  text. No undocumented game functions or clipboard use in the automatic path.
- Add Copiar resumen de daño as an explicit alternative for an unavailable game
  chat. It copies the actual snapshot; it does not send to another application.
- Replace all fourteen simplified silhouettes with original weapon icons extracted
  from the installed game's cmn_item00_ID.tex. Decode once with system WIC and
  cache the GPU bitmap; no extractor or decompressor is required at runtime.
- The user's failed 0.6.4 attempts occurred while testing alone, with no native
  online session and no manually opening chat. Actual chat delivery on 0.6.5 still
  requires testing in an online session where the game's normal chat works.

## 0.6.4 (candidate)

- Identify current damage and hunt history by quest name; record details also
  show objectives. Includes 521 offline quest entries from MIT MHWorldData,
  Spanish where present and English fallback. Unknown IDs remain Misión #ID.
  Existing IDMHST03 history remains compatible and gains labels without migration.
- Allow scale 40–175% and width 260–640; narrow rows stack names and numbers.
  History keeps mission, date, duration and personal damage/DPS on distinct lines.
  Long text uses ellipsis, and settings adapt to the available width.
- Add menu action Enviar daño actual al chat: explicitly requested sharing of
  a real snapshot, group total and each recorded player's damage/share. No preview
  data or automatic end-of-quest messages. Lines stay within 80 UTF-16 units;
  one request at a time, 15-second cooldown, focus-loss/Escape cancellation.
- Chat uses the game's standard Insert/Enter text entry and current channel,
  without calling undocumented game functions or changing clipboard contents.
  Set the game's chat-open key to Insert or Enter consistently in settings.
  Delivery with the real game remains to be confirmed in a mission.
- Add catalog/history/layout/chat tests and eight narrow/minimum-scale DX11/DX12
  GPU cases, including real pixels, resizes and state preservation.

## 0.6.3 (candidate)

- Avoid COM Release calls from global graphics and DirectInput owners during
  process teardown. Explicit reset/reassignment and local RAII still release
  resources normally; the pinned plugin's remaining resources belong to Windows
  at exit. This addresses the real 0.6.2 shutdown trace in the AMD driver through
  the bootstrap DX11 device's static destructor.
- Adds a process-exit COM regression test; all 28 local tests pass.
- Real-game startup, preview bars and normal exit passed on DX12 with and without
  Stracker, and on DX11 with Stracker + Seamless; Steam overlay stayed enabled.
- The user and a friend reported a successful multiplayer playtest with expected
  behavior. Exact damage accuracy, long sessions and broader hardware coverage
  remain unverified; not a stable release.

## 0.6.2 (WITHDRAWN; shutdown crash after successful DX12 startup/render)

- Replaces shared DXGI vtable writes with per-target MinHook trampolines.
- Each callback carries its own original trampoline, independent of other
  overlays replacing or copying object tables. No per-frame hook allocation.
- Four external-overlay ordering tests pass on DX11/DX12 (27 tests total).
- Includes MinHook BSD notices and sources; no extra runtime install required.
- 0.6.1 failed in the real game with Steam overlay and is withdrawn.
- On 2026-11-11, real-game DX12 startup and preview bars were verified with
  Steam overlay and Seamless, both with and without Stracker. Settings were
  visually verified in the combined-loader run. Gameplay, multiplayer accuracy
  and real-game DX11 remain pending. See docs/VALIDATION.md for limits.

## 0.6.1 (WITHDRAWN)

- Correct copied DXGI vtable handling: never register our callback as its own
  original. Resolve a unique compatible runtime original, or decline the patch.
- Added copied-table regressions for DX11/DX12, including late attachment.
- Regression against 0.6.0 reproduced 0xC00000FD; corrected fixtures pass.
- Real-game startup confirmation is still required before wider distribution.

## 0.6.0 — WITHDRAWN

Confirmed game startup crash (stack overflow 0xC00000FD). Do not distribute.
Recovery: disable our plugin and our chain DLL, preserving Seamless's root DLL.

- Automatic native DX11/DX12 rendering in the same plugin and manual-copy ZIP.
- DX11 late/early attachment, legacy/flip swapchains, cached Direct2D rendering,
  private BGRA device, GPU sharing and full game-context-state restoration.
- Shared texture synchronization has zero timeout; busy overlay work is skipped
  or retried next frame rather than stalling the game. UI-region scissoring.
- Added DX11 pixel-readback, pipeline-state, resizing and cache-reuse tests on
  WARP/hardware. Actual gameplay/FPS and multiplayer validation remain pending.
- Removed DX12-only instructions and documented Seamless loader diagnostics.
- Fixed Seamless loading: its loader filters foreign plugins. A dedicated
  Seamless ZIP uses its XInput chain without replacing the existing loader.

## 0.5.1

- Copy-to-game-folder distribution: root XInput bootstrap and nativePC.
- Removed installation/uninstallation/diagnostic scripts and installer fixtures.
- Short bilingual guide: existing Seamless/Stracker users copy only nativePC;
  keep other mods' XInput DLLs and existing settings when updating.
- Documentation and manifest are grouped under IceborneDamageMeter in the ZIP.
- No change to runtime logic or polling cadence.

## 0.5.0

- Self-contained clean installation: original MIT-licensed XInput bootstrap.
- Reuses existing Stracker/shared loaders and preserves unrecognized proxies.
- Verifies both DLLs, records bootstrap ownership and preserves external loaders
  during updates/uninstall. Bootstrap source and tests are included.
- Startup worker exits after one plugin load; XInput forwarding uses cached
  system exports. No additional polling or rendering in the bootstrap.
- First clean-install real-game test remains pending.

## 0.4.0

- Public distribution layout, MIT license and contribution documentation.
- Steam-library detection, manual game selection, non-interactive installation,
  diagnostics and integrity manifest; both compatible loader families recognized.
- One CMake build definition, versioned DLL metadata, presets, checksums and CI.
- New default settings key F6 avoids Seamless Co-op's F10; existing INIs preserved.
- No new per-frame runtime processing for the packaging changes.

## 0.3.1

- Compact bars and a single on-demand menu instead of persistent tabs and footer.

## 0.3.0

- Observed DPS, participation, ten-hunt history, comparisons and in-game settings.
- Cached panel composition and asynchronous persistence.

## 0.2.1

- Weapon colors visible at zero damage; relative editing cursor.

## 0.1–0.2

- Initial native DX12 damage meter, late attachment, weapon icons and dragging.
