# Iceborn Damage Meter — instalación y uso

[English](INSTALL.md) · [Créditos y licencias](THIRD_PARTY.md)

## Requisitos

Monster Hunter World: Iceborne, Windows 10/11 x64, DirectX 11 o 12 con SDR
y el ejecutable compatible con el perfil 421810. El manifiesto incluido indica
el hash compatible. No necesitás una aplicación de overlay ni herramientas de
desarrollo. El paquete estándar incluye nuestro cargador; Stracker no es
obligatorio para una instalación limpia.

## Instalar

1. Cerrá el juego. En Steam, elegí Administrar > Ver archivos locales.
2. Copiá la carpeta `nativePC` del ZIP junto a `MonsterHunterWorld.exe`.
3. Sin cargador previo, copiá también el `xinput9_1_0.dll` del ZIP estándar.
   Si ya usás Stracker, copiá solamente `nativePC` y conservá su cargador.

No reemplaces DLL de otro mod. Si ya hay un proxy XInput, revisá sus instrucciones
de carga. La variante de encadenamiento es para un cargador previo compatible:
se copian su `nativePC` y `xinput9_1_0.chain.dll`, conservando la DLL raíz existente.
Esa variante no sirve como instalación limpia. No reemplaces un archivo
`.chain.dll` de otro mod.

La carpeta `IceborneDamageMeter` contiene instrucciones, licencias y manifiesto;
no es una aplicación adicional. El ZIP de fuentes es para desarrollar.

## Actualizar o quitar

Cerrá el juego. Conservá `iceborne_damage_meter.ini` e
`iceborne_damage_meter_history.dat` e `iceborne_damage_meter_hunts.bin` en
`nativePC/plugins`; no los reemplaces con
los predeterminados del ZIP nuevo. Reemplazá solamente nuestro plugin y, cuando
una versión lo requiera, nuestro cargador correspondiente. Conservá los de otros mods.

Para quitar el medidor, borrá solo `nativePC/plugins/iceborne_damage_meter.dll`.
Podés conservar ajustes e historial. Quitá nuestro cargador únicamente si
confirmaste que pertenece al paquete y tu instalación no lo necesita.
No borres toda la carpeta `nativePC`.

Si el mod ya funciona, usá el ZIP **update-plugin-only** con el juego cerrado.
Reemplaza solo nuestro plugin y no incluye INI ni cargador. La 0.6.14 no requiere
reemplazar el bootstrap del medidor 0.6.11 que ya funciona. Para una instalación
nueva, usá el paquete completo siguiendo las instrucciones de arriba.

## Controles y tamaño

El panel aparece automáticamente durante una misión. Controles predeterminados:

| Tecla | Acción |
|---|---|
| F8 | Mostrar/ocultar |
| Ctrl+F8 | Vista previa con datos de ejemplo |
| F6 | Ajustes |
| Ctrl+F6 | Menú y portapapeles |
| F7 | Historial de cacerías |
| Ctrl+F9 | Desbloquear/bloquear movimiento |

Se conservan las teclas configuradas en tu INI. Podés ajustar escala 40–175%,
ancho 260–640, opacidad, filas y columnas. Se guardan ajustes y diez cacerías.
El DPS incluye el tiempo presente, con viajes y pausas; no mide tiempo de ataque
activo ni una puntuación de habilidad. En misiones con varios objetivos, el daño
corresponde a la misión completa.

F6 también permite cambiar **Mostrar botón DM** y **Mostrar al iniciar misión**.
Desactivá la primera para quitar el pequeño acceso DM cuando ocultás el panel.
Desactivá la segunda para iniciar oculto y manejarlo con la tecla entre misiones.
Ambas opciones se guardan. Con las dos apagadas, F8 (o tu tecla configurada)
sigue abriendo el panel; F6, F7 y Ctrl+F6 abren sus páginas. Se sigue registrando
daño e historial mientras está oculto, sin dibujar el panel ni el botón en la GPU.

Opciones equivalentes bajo `[Overlay]` en el INI: `ShowBadge=0` y `AutoShow=0`.

## Archivo de cacerías a largo plazo

`ArchiveHunts=1` bajo `[Overlay]` activa el archivo
`nativePC/plugins/iceborne_damage_meter_hunts.bin`. Agrega resúmenes en segundo
plano para un futuro analizador externo; el medidor sigue mostrando diez
cacerías y no carga ni analiza el archivo completo. Al iniciar importa los
registros posteriores a la última entrada común con el archivo; si no hay
coincidencias, importa los registros recientes disponibles.

Con el juego cerrado, poné `ArchiveHunts=0` para desactivar el guardado acumulativo.
Conservá el archivo al actualizar. Para quitarlo, cerrá el juego y borrá solamente
ese archivo; el historial de diez es independiente. Los errores se registran
en el log y no desactivan el historial reciente. Una salida repentina puede perder
datos que todavía estén en la cola.

## Idioma

`Language=auto` bajo `[Overlay]` en el INI usa el idioma del juego configurado
en Steam. Reiniciá el juego después de cambiarlo. Opciones manuales:
`en`, `es`, `pt-BR`, `fr`, `de`, `it`, `ja`, `ko`, `zh-CN`, `zh-TW`, `ru`, `pl`, `ar`, `es-419`.
El español latinoamericano usa sus textos de misión propios y comparte etiquetas
de interfaz en español. Se incluyen nombres/objetivos oficiales de 512 IDs del
juego en PC, también en chino simplificado. Las entradas antiguas sin coincidencia
conservan inglés como respaldo; las misiones desconocidas siguen numeradas.
Se conservan los nombres de jugadores. El panel compacto mantiene una fila por
cazador hasta ancho 260; las páginas detalladas conservan su diseño estrecho.

## Copiar resumen de daño

Abrí el menú y elegí **Copiar resumen de daño** para llevar el informe de la
misión actual al portapapeles. Los totales siguen disponibles al volver a
ciudad hasta que otra misión los reinicie. No se copian datos de vista previa
ni misiones sin daño. No se envía nada al chat del juego.

Se retiró el envío por chat. Las entradas antiguas `ChatTransport` y
`ChatOpenKey` del INI se ignoran; no hace falta modificar tu configuración.

## Limitaciones y problemas

No se mide daño en expediciones ni Tierras Destino: se muestra sin datos.
Las investigaciones con ID genérico 30000 se identifican como Investigación,
sin inventar objetivos ni compararlas como la misma cacería. Las misiones
desconocidas conservan su número. HDR y recuperación de pérdida del dispositivo
gráfico no están soportados. No se verificaron todas las combinaciones de GPU,
idioma, tamaño de grupo o cargadores.

Si no aparece el panel, probá Ctrl+F8. Revisá
`nativePC/plugins/iceborne_damage_meter.log` y el log del cargador junto al EXE.
Para reportar un crash gráfico, incluí GPU/controlador, API, versión del mod
y pasos para reproducirlo. Quitá datos personales de logs y capturas.

Descargas y reportes: https://www.nexusmods.com/monsterhunterworld/mods/8969
