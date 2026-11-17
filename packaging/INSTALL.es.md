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
`iceborne_damage_meter_history.dat` en `nativePC/plugins`; no los reemplaces con
los predeterminados del ZIP nuevo. Reemplazá solamente nuestro plugin y, cuando
una versión lo requiera, nuestro cargador correspondiente. Conservá los de otros mods.

Para quitar el medidor, borrá solo `nativePC/plugins/iceborne_damage_meter.dll`.
Podés conservar ajustes e historial. Quitá nuestro cargador únicamente si
confirmaste que pertenece al paquete y tu instalación no lo necesita.
No borres toda la carpeta `nativePC`.

## Controles y tamaño

El panel aparece automáticamente durante una misión. Controles predeterminados:

| Tecla | Acción |
|---|---|
| F8 | Mostrar/ocultar |
| Ctrl+F8 | Vista previa con datos de ejemplo |
| F6 | Ajustes |
| Ctrl+F6 | Menú y envío manual |
| F7 | Historial de cacerías |
| Ctrl+F9 | Desbloquear/bloquear movimiento |

Se conservan las teclas configuradas en tu INI. Podés ajustar escala 40–175%,
ancho 260–640, opacidad, filas y columnas. Se guardan ajustes y diez cacerías.
El DPS incluye el tiempo presente, con viajes y pausas; no mide tiempo de ataque
activo ni una puntuación de habilidad. En misiones con varios objetivos, el daño
corresponde a la misión completa.

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
`en`, `es`, `pt-BR`, `fr`, `de`, `it`, `ja`, `ko`, `zh-CN`, `zh-TW`, `ru`, `pl`, `ar`.
Los títulos de misiones sin traducción aparecen en inglés. Se conservan los
nombres de jugadores.

## Compartir daño

Abrí el menú y elegí enviar el daño actual al chat. Los totales de la misión
terminada siguen disponibles al volver a ciudad hasta que otra misión los reinicie.
No se comparte daño vacío ni de vista previa. Copiar resumen permite llevar
el informe al portapapeles mediante una acción explícita.

Los ajustes nuevos usan `ChatTransport=keyboard` bajo `[Overlay]`, con escritura
pausada. `ChatOpenKey=45` corresponde a Insert; `13`, a Enter. Usá la tecla
asignada al chat del juego. Para el envío directo experimental sin escritura,
cerrá el juego y poné `ChatTransport=native` en esa sección. Conservá el resto
del INI. Ese modo protege otros borradores, no reintenta automáticamente ni
cambia al transporte por teclado si falla.

Usá una sesión online y el canal deseado. Esperá al menos 15 segundos entre
solicitudes. Un grupo largo puede requerir varios mensajes. Confirmá recepción
con otro jugador: el estado local no garantiza entrega. Solo se comparte
cuando lo pedís.

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
