# Referencias técnicas

[English primary notices](THIRD_PARTY.md). Los avisos por versión de abajo son
antecedentes; el documento inglés contiene la referencia principal vigente.

Referencias adicionales para 0.6.7:
- Stage ID y campo ZoneOffsets del perfil HunterPie 421810 (Apache-2.0):
  https://github.com/HunterPie/HunterPie/blob/main/HunterPie.Core/Game/Enums/Stage.cs
  https://github.com/HunterPie/HunterPie/blob/main/HunterPie/Address/MonsterHunterWorld.421810.map
- Diseño de campos de chat 15.23 de mhw-toolkit, Copyright 2024 Eigeen,
  Apache-2.0. Se usan como referencia para una implementación propia acotada;
  no se enlaza ni redistribuye su biblioteca. Licencia conservada en
  third_party/mhw-toolkit/LICENSE.txt y paquetes:
  https://github.com/eigeen/mhw-toolkit/blob/main/src/game_export.rs
  https://github.com/eigeen/mhw-toolkit/blob/main/src/game_util.rs
- Limitación y métodos alternativos de daño de expedición:
  https://github.com/gabrielefilipp/SmartHunter

El código del medidor es MIT. Desde 0.6.2 incorpora MinHook v1.3.4 enlazado
estáticamente, bajo BSD de dos cláusulas. Código y avisos en third_party/minhook;
la licencia también se incluye en los paquetes binarios. Fuente oficial:
https://github.com/TsudaKageyu/minhook/tree/v1.3.4

La disposición de contadores, jugadores y estados de misión se consultó en el
proyecto HunterPie (Apache-2.0), especialmente:

- https://github.com/HunterPie/HunterPie/blob/main/HunterPie/Address/MonsterHunterWorld.421810.map
- https://github.com/HunterPie/HunterPie/blob/main/HunterPie.Integrations/Datasources/MonsterHunterWorld/Entity/Player/MHWPlayer.cs
- https://github.com/HunterPie/HunterPie/blob/main/HunterPie.Integrations/Datasources/MonsterHunterWorld/Definitions/MHWPartyMemberStructure.cs
- https://github.com/HunterPie/HunterPie/blob/main/HunterPie.Integrations/Datasources/MonsterHunterWorld/Definitions/MHWQuestStructure.cs
- https://github.com/HunterPie/HunterPie/blob/main/HunterPie.Integrations/Datasources/MonsterHunterWorld/Entity/Enums/QuestState.cs

El lector utiliza exclusivamente campos de datos documentados por esas fuentes;
no incluye sus funciones de parcheado o código de bypass.

Renderizado basado en las APIs de Windows documentadas por Microsoft:
https://learn.microsoft.com/en-us/windows/win32/direct3d12/d2d-using-d3d11on12

Desde 0.5.0 el paquete incluye un bootstrap original (bootstrap/bootstrap.cpp y
bootstrap/exports.def), bajo la misma licencia MIT del proyecto. Reenvía las
cuatro funciones de XInput 9.1.0 al DLL del sistema mediante una ruta absoluta y
carga únicamente nuestro medidor. No redistribuye el DLL de Microsoft.

Stracker's Loader puede cargar el plugin directamente. MHW mod loader filtra
plugins ajenos: la integración con Seamless usa su cadena XInput para iniciar
nuestro bootstrap como xinput9_1_0.chain.dll. No se incluyen sus DLL ni se copió
su cargador. Las referencias a esos
proyectos describen compatibilidad y no los incorporan a nuestra licencia MIT.

APIs de carga y XInput documentadas por Microsoft:
- https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-versions
- https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryexw

La ruta DX11 usa un dispositivo BGRA propio, textura GPU compartida y restauración
del estado de contexto; las APIs están documentadas por Microsoft:
- https://learn.microsoft.com/en-us/windows/win32/direct2d/devices-and-device-contexts
- https://learn.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-swapdevicecontextstate
- https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgikeyedmutex-acquiresync

Colores: referencia de tonos de iconos por arma en MHW:I (tarjeta de gremio),
https://wikiwiki.jp/nenaiko/ゲーム用語/アイコン#武器アイコンの色 . Los RGB del HUD son
adaptaciones para contraste, no colores oficiales. Hasta 0.6.4 se usaban siluetas
originales en weapon_icons.hpp. Desde 0.6.5 las armas válidas usan los iconos
originales del juego, separados de la licencia MIT del código (ver abajo).

Entrada del modo de edición, APIs Win32 de Microsoft:
- https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerrawinputdevices
- https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getregisteredrawinputdevices
- https://learn.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawinputdevice

El registro temporal del mouse se limita al modo de edición y se restaura al salir.

## Catálogo de misiones (0.6.4)

Nombres y objetivos derivados de gatheringhallstudios/MHWorldData, licencia MIT,
Copyright (c) 2018 Carlos Fernandez. Revisión fijada:
be7362213d7d1e30b794e3b58d3f87712035658d.
https://github.com/gatheringhallstudios/MHWorldData

La fuente contiene third_party/mhworlddata/quests.csv, README.md y LICENSE.txt;
scripts/generate-quest-catalog.py genera src/quest_catalog.hpp sin red.
Los ZIP runtime incluyen licenses/MHWorldData-LICENSE.txt en IceborneDamageMeter.
Solo se incorporan ID, nombre y objetivo, sin imágenes ni descripciones extensas.

## Iconos originales de armas (0.6.5)

Los catorce iconos proceden de ui/common/tex/cmn_item00_ID.tex, extraído del
chunkG0.bin de la instalación local del usuario. El recorte y el mapeo por tipo
nativo están documentados en assets/game_weapon_icons.json. Copyright CAPCOM;
el aviso Game-artwork-NOTICE.txt distingue estos gráficos del código MIT.

El PNG se incorpora a src/game_weapon_icons.hpp mediante scripts/embed-weapon-icons.py.
Se decodifica una sola vez con WIC de Windows y se conserva en una textura GPU.
Las herramientas de inspección/extracción permanecen exclusivamente en build;
no se distribuyen ooz, MHWNoChunk, un atlas completo ni sus DLL o dependencias.
