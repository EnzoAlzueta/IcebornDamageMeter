# Contributing

Use English for identifiers, comments, diagnostics, tests, primary documentation,
issues and pull requests. Keep user-facing translations in locales/strings.json;
see [localization](locales/README.md). Spanish companion guides use `.es.md` and
link to their English counterpart. Do not translate existing INI keys or history
identifiers, since that would break persisted settings or data.

Build and run tests using docs/BUILDING.md. Keep game reading, statistics,
persistence, presentation and packaging responsibilities separate.

Preserve the 4 Hz data cadence, bounded history/player lists, cached text and
geometry, and background writes. Do not add file/network work in the per-frame
render path. Report profiling conditions for performance claims.

New game profiles require evidence that their offsets match the supported
executable. Do not remove the executable-hash guard to make an unsupported version
appear supported. Unknown data should stay unknown rather than fabricate damage.

Add tests for meaningful state, parsing or loading behavior. Local DX11/DX12
tests and real multiplayer verification are separate from headless CI results.
Do not commit game files, third-party loaders, user configuration, logs or hunt data.
