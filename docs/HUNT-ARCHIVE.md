# Hunt archive format v1

Starting with 0.6.12, `ArchiveHunts=1` under `[Overlay]` enables a local cumulative
file named `nativePC/plugins/iceborne_damage_meter_hunts.bin`. Set the option to
`0` and restart the game to disable new archive writes. Disabling it does not
delete existing data. Nothing is uploaded or analyzed by the meter.

The normal `iceborne_damage_meter_history.dat` format and ten-entry UI remain
unchanged. The archive is independent: its size never increases that history's
memory, sorting or rendering work. The existing last-hunt text summary remains.

## Operation and cost boundaries

- Only finalized/interrupted hunt summaries are queued, not individual hits or
  samples. Serialization, archive reads/writes and flushing run on the storage
  worker, never in the frame drawing routine.
- The producer adds one record to a queue with 32 slots, independent of archive
  size. It does not wait for disk I/O. The existing ten-record snapshot is still
  bounded. An exhausted queue logs missing records instead of growing memory or
  blocking rendering; this is an overload condition, not a normal hunt cadence.
- On startup, read a 24-byte header and at most 28,523 bytes from the end of the
  archive. Retain at most ten encoded records for duplicate checks. No complete
  archive scan or statistics calculation occurs in-game.
- On startup, find the newest byte-identical record shared by the ten-entry
  history and archive tail cache, then import newer recent records in chronological
  order. With no common record, import the available recent snapshot. This also
  recovers recent records after a previous interrupted save without duplicating
  an older history snapshot when the archive was already ahead. It is not a
  global deduplication database; restoring older backups can reintroduce records
  outside that small cache. Records discarded before this version are unavailable.
- New frames append to the existing file. Each normal hunt append flushes from
  the worker. Initial recent-record imports share one final flush. No whole-file
  rewrite, UI archive browser, aggregate, network service or JSON dependency is added.
- File/schema/write errors disable only the archive for the session, preserving
  recent history and settings. The log identifies the failure. A process crash
  or immediate exit can still lose a summary that has not reached the worker.

Compare performance only under stated conditions. The synthetic benchmark uses
the filesystem cache and does not establish in-game FPS or absence of stutter.

## File header: 24 bytes

All integer fields are little-endian. There is no compiler struct padding.

| Offset | Type | Meaning |
|---:|---|---|
| 0 | 8 bytes | ASCII `IDMHARC1` |
| 8 | uint32 | Schema version, `1` |
| 12 | uint32 | Game memory profile/build, `421810` |
| 16 | uint32 | Maximum payload size, `2573` |
| 20 | uint32 | CRC-32/ISO-HDLC of bytes 0–19 |

CRC uses reflected polynomial `0xEDB88320`, initial value `0xFFFFFFFF`, final XOR
`0xFFFFFFFF`. The checksum of ASCII `123456789` is `0xCBF43926`. CRC detects
accidental damage; it is not cryptographic proof of authenticity.

## Record frame: payload size + 20 bytes

| Offset | Type | Meaning |
|---:|---|---|
| 0 | 4 bytes | ASCII `HNT1` |
| 4 | uint32 | Payload byte length `N`, 525–2573 |
| 8 | N bytes | Hunt payload described below |
| 8+N | uint32 | CRC-32 of the payload |
| 12+N | uint32 | Total frame length `N+20` |
| 16+N | 4 bytes | ASCII `END1` |

The framing permits bounded startup recovery. Only a structurally recognized,
incomplete trailing frame is truncated back to the preceding valid frame (or
header). Complete corrupt frames, unknown headers and foreign trailing bytes
are preserved and disable archiving. Existing interior frames are not scanned;
an external analyzer should validate every frame it reads.

## Hunt payload

The payload matches one record of the existing `IDMHST03` history codec, without
that history file's magic or record count.

| Type | Meaning |
|---|---|
| uint64 | Capture timestamp, Unix UTC milliseconds |
| uint64 | Observed hunt duration, milliseconds |
| int32 | Quest ID |
| int32 | Outcome/state; 3/4 are treated as success by the meter; 6 marks interruption |
| uint32 | Peak simultaneous party size, 0–4 |
| uint8 | Partial-observation flag, 0/1 |
| 16 player entries | Fixed slot count, including unused entries |

Each player entry contains these fields in order:

| Type | Meaning |
|---|---|
| uint64 | Player ID as observed by the reader |
| uint32 | Recorded damage |
| uint32 | Damage already present at first observation |
| uint64 | Observed participation time, milliseconds |
| int32 | Native weapon type, 0–13; 255 unknown |
| uint8 | Flags: bit 0 local, bit 1 present, bit 2 weapon changed |
| uint16 | Name length in UTF-16 code units, 0–64 |
| uint16 × length | UTF-16LE name, no terminator |

A fixed player entry occupies 31 bytes before its name. The hunt header is 29
bytes, so payload size is `525 + 2 × sum(name lengths)`. Four twelve-unit names
give 621 payload bytes, 641 framed bytes: about 64.1 MB for 100,000 such hunts,
plus the 24-byte file header. Longer names and departed participants increase size.

Player slots include departed participants; party size describes the peak
simultaneous group, not the count of nonempty slots. Derived DPS is
`1000 × (damage - baselineDamage) / participationMs`, with zero time handled
explicitly. It includes travel/downtime while present. Do not infer active attack
time, build quality, per-monster attribution or a universal weapon tier list.
Generic investigation ID 30000 does not identify a stable target/quest variant.
Partial observations and weapon changes should be excluded from like-for-like
comparisons. Names/IDs are local data; review them before publishing an archive.
