# Statistics and performance model

Damage is the absolute party counter read from the supported game profile. It
is sampled at 4 Hz and is never summed repeatedly. Team percentage divides a
player's counter by the sum of all recorded players, including departed players.

Observed DPS is `(current damage - first observed damage) / participation time`.
The time includes downtime while the player is present. It freezes when the
player leaves or the quest ends; final damage synchronization may arrive later.
It is not active attack time, a server combat log or a replacement for the game's
quest timer. A player with inherited damage does not get that damage divided by
the short time since the plugin first observed them.

Personal comparison uses the previous successful record with the same local
name, quest ID, weapon and peak party size. Known partial observations, prior
damage and weapon changes are excluded. Different quests sharing a monster are
not equated. Multi-target damage is mission-wide. These are descriptive metrics,
not a normalized player skill score.

The history is capped at ten records; player storage is bounded. Text and panel
commands are reused between changes. Hidden panels do not rebuild their rows.
Persistence runs on a sleeping worker, including settings updates. No network or
file operations are added to the per-frame drawing routine.

The bundled bootstrap loads only the meter once, then its startup thread exits.
XInput calls pass through cached pointers to Windows; the bootstrap adds no
periodic polling, graphics work, network activity or resident worker thread.

DX11 uses a reusable private BGRA device and shared texture on the game's GPU.
The game device need not have BGRA enabled. Cached commands draw into the UI
region, with region-limited clearing and scissored alpha composition. Game
pipeline state is restored through context-state swapping. No CPU pixel transfer
or per-frame resource/shader creation is used. Shared keyed-mutex acquisition
uses a zero timeout: busy UI work is skipped/retried without a GPU wait loop.
The extra device/shared texture consumes memory; real game cost still needs
measurement on representative hardware. DX12 keeps its existing On12 renderer.

Build reproducibility here means a shared, documented build procedure. Identical
binary bytes across different compiler/SDK versions are not guaranteed. In-game
FPS and full multiplayer correctness remain separate field tests.
