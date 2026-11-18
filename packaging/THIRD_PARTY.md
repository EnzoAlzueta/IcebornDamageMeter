# Credits and third-party notices

Original project code: Copyright (c) 2026 Enzo and contributors, under the
[MIT License](LICENSE). That license does not cover CAPCOM artwork or relicense
third-party components.

## Included components and data

- MinHook v1.3.4, Tsuda Kageyu and contributors, statically linked under BSD
  2-Clause. [License](licenses/MinHook-LICENSE.txt).
  https://github.com/TsudaKageyu/minhook/tree/v1.3.4
- Quest names/objectives: MHWorldData, Copyright (c) 2018 Carlos Fernandez,
  MIT. [License](licenses/MHWorldData-LICENSE.txt). Catalog revision:
  `be7362213d7d1e30b794e3b58d3f87712035658d`.
  https://github.com/gatheringhallstudios/MHWorldData
- Official localized quest names/objectives: CAPCOM game text, separate from
  the code and legacy CSV licenses. [Game notice](licenses/Game-artwork-NOTICE.txt).
  The source package records text coverage and extraction provenance. Raw game
  archives, GMD files and extraction tools are not bundled.
- Fourteen original weapon icons: CAPCOM game artwork, separate from the code
  license. [Artwork notice](licenses/Game-artwork-NOTICE.txt). Extraction metadata
  is available in the developer source package. Attribution is not a grant of
  permission to redistribute that artwork.

## Technical references

- HunterPie, Apache-2.0: game memory layouts for player, party, quest and stage
  fields. Its bypass/patch routines are not included.
  https://github.com/HunterPie/HunterPie
- mhw-toolkit, Copyright 2024 Eigeen, Apache-2.0: historical native chat field layouts (chat sending removed in 0.6.14).
  The toolkit library is not bundled or linked.
  [Reference license](licenses/mhw-toolkit-REFERENCE-LICENSE.txt).
  https://github.com/eigeen/mhw-toolkit

The included bootstrap is original project code. No third-party loader,
extraction utility, full game atlas or decompressor is bundled.
