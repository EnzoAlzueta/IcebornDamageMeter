# Release plan

[Español](PLAN-LANZAMIENTO.es.md)

## Current status — 2026-11-15

0.6.11 is installed for the user and friend. The user reports successful testing
and no further crashes. All 46 local tests passed before that report. The tested
archives are frozen without rebuilding, with matching publication materials in
dist/publication-0.6.11. The original ERR12 cause is not confirmed; unreported
scenarios remain unverified. The author reports publishing to
https://www.nexusmods.com/monsterhunterworld/mods/8969 on 2026-11-15.
The web tool could not access the page; remote content and downloads were not
independently verified. Preserve the frozen archives and collect user reports.
No external page or upload was modified by Codex.

## Historical preparation checklist before 0.6.11

0.6.9 is installed and its latest investigation/name/town-sharing fixes were
user-tested. 0.6.10 standardizes English source and documentation; it is a separate
prepared candidate. Freeze features while preparing a public beta. Expedition
damage is deferred by the user's explicit decision.

1. Complete local validation of 0.6.10 and a real-game smoke test after installing
   it: correct language, unchanged labels/data, rendering, normal shutdown.
2. Test clean installation, upgrade and removal using only the published guide.
   Keep INI/history and existing loaders; distinguish normal/Stracker/Seamless.
3. Record actual results for normal quests, arena, events, investigation, town
   sharing, repeated hunts and party changes. Unknown scenarios stay unverified.
   Do not generalize two-player results to four players or every GPU/API/loader.
4. Review translations with speakers when possible and inspect long text at
   minimum panel width/scale. Automated coverage does not prove language quality.
5. Review licenses and redistribution of extracted game artwork before release;
   prepare original alternatives if authorization cannot be established.
6. Run the release candidate's tests, translation audit and package checks on
   the exact artifacts to be shared. Include normal/Seamless/source ZIPs and hashes.
   Exclude local research tools, private logs, user data, backups and fixtures.
7. Prepare screenshots, download links, supported scope and a bug-report channel.
   If a repository is created, complete its first actual hosted CI run before
   claiming hosted build verification. Use the [release draft](RELEASE-BETA.md).
8. Publish only to the destination chosen and authorized by the user. Start with
   a beta, collect external reports and resolve blocking accuracy, crash or data
   loss problems before calling a version stable. No version number or deadline
   substitutes for evidence.

Local tests and historical successful playtests do not replace the remaining
field matrix. No external publication or account change is authorized by this file.
