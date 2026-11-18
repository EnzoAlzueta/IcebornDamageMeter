# Localization

English is the source language of the code and public documentation. In
`strings.json`, each English key maps to 12 values in the exact order of
`languages`: Spanish, Portuguese, French, German, Italian, Japanese, Korean,
Simplified Chinese, Traditional Chinese, Russian, Polish and Arabic.

Use `tr(L"English source text")` for visible interface text. English returns the
source directly; other languages use the generated table. Preserve whitespace
and printf placeholders. Do not translate configuration keys, player names or
stored identifiers. Add every translation when introducing a source string.

Run `python scripts/check-localization.py` to regenerate
`src/translations_data.hpp` and verify source coverage and format placeholders.
Commit both JSON and generated header. Normal builds and players need no Python
or network translation service. Quest names/objectives are a separate offline
catalog; see `third_party/mhworlddata/README.md`.
The catalog now prefers official game text from `assets/quest_text`. Latin
American Spanish is a fourteenth game-text locale (`es-419` / Steam `latam`),
reusing Spanish UI strings, so the UI translation table still has twelve columns.

Write code identifiers, comments, diagnostics, tests, issues and primary guides
in English. Use `.es.md` for Spanish companion documents, linked from the English
guide. Existing translations are user-facing content, not text to anglicize.
