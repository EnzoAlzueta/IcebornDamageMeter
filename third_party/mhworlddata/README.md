# Quest names and objectives

Derived from gatheringhallstudios/MHWorldData, MIT, Copyright (c) 2018 Carlos Fernandez.
Upstream commit: be7362213d7d1e30b794e3b58d3f87712035658d.
Source: https://github.com/gatheringhallstudios/MHWorldData/blob/be7362213d7d1e30b794e3b58d3f87712035658d/source_data/quests/quest_base_translations.csv

quests.csv contains the numeric quest ID and names/objectives in the upstream
English, Spanish, Portuguese, French, German, Italian, Japanese, Korean,
Traditional Chinese, Russian, Polish and Arabic columns.
Descriptions and unrelated columns are omitted. The compiled index follows the selected UI language and uses English for
missing translations. Upstream Chinese is Traditional; Simplified Chinese quest
titles/objectives explicitly use English. Bidi control characters are stripped
from generated literals; paragraph direction is handled by the renderer. It is generated offline by
scripts/generate-quest-catalog.py; players need neither Python nor network access.

Coverage is the upstream catalog, not every custom quest or procedural investigation.
Unknown IDs remain explicitly identified as Misión #ID, never assigned a guessed name.
Existing hunt history already stores quest IDs and needs no format migration.
See LICENSE.txt for the upstream MIT notice.
