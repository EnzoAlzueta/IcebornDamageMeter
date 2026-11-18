# Official localized quest names and objectives

game_quests.csv contains only quest names/objectives extracted offline from the
author's installed Monster Hunter World: Iceborne text resources. It includes
Simplified Chinese and the game's separate Latin American Spanish text. These
are official game strings, not AI translations or Traditional-to-Simplified conversion.

Original game text is copyright CAPCOM, separate from the MIT code license.
See ../Game-artwork-NOTICE.txt. Raw GMDs, chunk files, extractors, decryption keys
and decoder DLLs are not included in distributions. provenance.json records
source profile, extraction date, catalog hash and aggregate source-manifest hash.

There are 512 usable official PC quest IDs, each with a title and objective in
all fourteen game-text locales. Placeholder Invalid Message/Unavailable rows
are omitted. Whitespace is flattened for single-line UI. The generator strips
bidi controls and resolves two Arabic gender-selection tags to their first
official grammatical variant; it does not read the player's gender.

scripts/generate-quest-catalog.py gives official IDs precedence over the legacy
MIT MHWorldData catalog. Older database IDs can reuse official localized strings
only when their exact English title and objective identify identical text.
The legacy catalog retains its previous English fallback where official text is
unavailable, including some platform-specific or obsolete quest IDs. Unknown
custom quests remain a localized Quest #ID, never an invented quest name.

Generic investigation ID 30000 contains Unavailable placeholders in the installed
GMDs. Its live procedural target is not established by this static text. The
meter continues to show the translated Investigation label with no guessed target.

All text is compiled into the existing lookup table. Players need no extraction
tool, Python, network lookup or per-frame file reads. Existing history/archive
formats remain unchanged: their quest IDs select the current localized label.
