# Instalación de Iceborne Damage Meter

[English installation guide](INSTALL.md). La candidata 0.6.10 estandariza código
y documentación en inglés, conservando los idiomas de la interfaz. Los apartados
por versión de abajo documentan los cambios previos. Estado vigente: README.md.

## Candidata 0.6.9

Investigaciones con ID genérico 30000 aparecen como Investigación, también en
historial. No se les asigna un objetivo inventado ni se comparan como si todas
fueran la misma cacería. Los nombres completos se conservan al compartir cuando
caben por entrada; un grupo con nombres largos puede ocupar varias líneas.

El chat directo admite sustituir su propio mensaje anterior exacto si el juego
ya consumió la solicitud. No reemplaza borradores distintos ni mensajes editados.
Probar dos envíos separados al menos 15 segundos: uno en misión y otro al volver
a ciudad manteniendo sesión online. Confirmar recepción con el amigo.

## Candidata 0.6.8: arco y envío sin escritura simulada

Corrige los iconos intercambiados de arco y ballesta ligera. La ballesta pesada
conserva su icono. Para probar el envío directo ya incluido desde 0.6.7, cerrar
el juego y usar `ChatTransport=native` bajo `[Overlay]` en el INI del plugin.
El ZIP conserva `keyboard` como predeterminado: mantener el INI propio al actualizar.

Con native, pulsar una vez Enviar daño actual al chat dentro de una sesión online,
sin tener un borrador abierto. No debe abrirse el chat ni escribirse letra a letra.
Confirmar con el amigo que recibió el resumen completo. El estado final no acredita
recepción. Si falla no vuelve automáticamente al teclado ni reintenta; revisar log
y usar Copiar resumen. Sigue siendo experimental, no validado online.

## Cambios preparados en 0.6.7 (todavía no instalada)

En expedición se limpia el daño de la misión anterior y se muestra
**EXPEDICIÓN · SIN DATOS**. El contador actual del juego no permite medir daño
fiable del grupo en expediciones, incluidas Tierras Destino: ese soporte sigue
pendiente. El historial anterior permanece accesible. Arena y eventos conservan
el flujo de misiones; hay regresiones locales, falta probar esos modos jugando.

Compartir usa un mensaje compacto para hasta cuatro jugadores: nombres
abreviados y daño exacto, sin título de misión ni porcentajes. **Copiar resumen
de daño** conserva el informe completo. Si hubo más de cuatro participantes,
puede necesitar varias líneas. El teclado escribe con pausas para evitar pérdidas.

Alternativa experimental sin teclado: con el juego cerrado, añadir
`ChatTransport=native` en `[Overlay]` del INI del plugin. Solo sirve con el
ejecutable compatible. No abre el chat ni cambia el canal seleccionado; rechaza
un borrador o entrada de texto activa. No reintenta ni pasa al teclado si falla.
Volver a `ChatTransport=keyboard` restaura el método normal. No es necesario
reemplazar tu INI al actualizar. Esta alternativa todavía no fue validada con
un receptor real: comprobarla con un amigo en sesión online. El aviso final no
confirma entrega. Después de enviar una solicitud nativa no puede retirarse.

Para probar 0.6.7: entrar en expedición después de una misión, verificar que no
aparezcan los totales anteriores, volver a misión/arena/evento y comprobar daño
nuevo. Probar un clic de compartir con daño real y confirmar que el amigo recibe
una línea completa. No sustituir DLL mientras esté abierto el juego.

## Idioma automático (candidata 0.6.6)

Con `Language=auto` en `[Overlay]`, el medidor toma el idioma del juego configurado
en Steam. Si la API todavía no está lista, usa el manifiesto de MHW, el idioma
de Windows o inglés, y reintenta durante el primer minuto. No modifica Steam ni
el juego. El cambio de idioma del juego puede requerir reiniciarlo.

Idiomas de interfaz: español, inglés, portugués, francés, alemán, italiano,
japonés, coreano, chino simplificado/tradicional, ruso, polaco y árabe.
Para elegir uno manualmente: `Language=es`, `en`, `pt-BR`, `fr`, `de`, `it`,
`ja`, `ko`, `zh-CN`, `zh-TW`, `ru`, `pl` o `ar`. Conservá tu INI al actualizar;
si no tiene esta opción, el valor predeterminado es `auto`.

Los títulos y objetivos usan las traducciones disponibles; si falta alguna,
aparecen en inglés. El catálogo no tiene títulos en chino simplificado.
Los nombres de jugadores y los registros guardados no se traducen ni modifican.
0.6.6 queda preparada para probar; no reemplaza automáticamente la instalación.

## Copiar y jugar

1. Cerrá el juego. En Steam: **Administrar > Ver archivos locales**. Se abre la
   carpeta que contiene `MonsterHunterWorld.exe`.
2. Abrí el ZIP para tu instalación y copiá su carpeta `nativePC` ahí, combinándola
   con la existente si ya la tenés.
3. **Sin cargador previo:** copiá también `xinput9_1_0.dll` junto al EXE.
   **Con Stracker:** copiá solamente `nativePC`; conservá tu cargador.
   **Con Seamless:** usá el ZIP **Seamless-windows-x64** y copiá también
   `xinput9_1_0.chain.dll` junto al EXE. Conservá el `xinput9_1_0.dll` de Seamless.
4. Podés usar **DirectX 11 o 12**: se detecta automáticamente. Usá salida SDR.
5. Entrá a una misión: el medidor aparece automáticamente. F8 lo muestra a mano.

No hay instalador, comando ni programa adicional que ejecutar. No necesitás
Python ni Visual Studio. El ZIP de código fuente es para desarrollar; para jugar
usá el ZIP windows-x64.

En una instalación sin otros mods queda así:

```text
Monster Hunter World/
  MonsterHunterWorld.exe            (ya existe; no viene en el ZIP)
  xinput9_1_0.dll
  nativePC/
    plugins/
      iceborne_damage_meter.dll
      iceborne_damage_meter.ini
```

La carpeta `IceborneDamageMeter` del ZIP contiene documentación, licencia y
manifiesto; el archivo `README-IceborneDamageMeter.txt` resume estos pasos.
Ninguno es necesario para que se dibujen las barras.

## Si ya hay un xinput9_1_0.dll

**No lo reemplaces por el nuestro.** Con Stracker, copiá solo `nativePC`.
Con Seamless usá el ZIP específico: nativePC más nuestro `.chain.dll`.
Si ese archivo pertenece a otro mod y no sabés si carga plugins,
consultá sus instrucciones de compatibilidad antes de agregar nuestro arranque.
El nombre del archivo por sí solo no identifica un cargador compatible. No hay
detección ni reemplazo automático, y nuestro arranque no encadena otros proxies.
Si ya existe `xinput9_1_0.chain.dll` de otro mod, tampoco lo reemplaces; hace falta
revisar esa combinación. No agregues otro cargador a ciegas.

El cargador de Seamless puede ignorar nuestro plugin al buscar directamente en
nativePC/plugins: el log dice `not one of these mods, left alone`. La integración
mediante .chain.dll carga nuestro arranque por otra vía, sin reemplazar Seamless.

Nuestro archivo es un bootstrap original MIT: carga únicamente el medidor y
reenvía las funciones del mando a Windows. No reemplaza las dependencias de otros
mods. Su hilo termina después del intento de carga.

## Actualizar o quitar

Para actualizar, con el juego cerrado reemplazá solamente
`nativePC/plugins/iceborne_damage_meter.dll`. Conservá tu INI para mantener los
ajustes. Si una versión requiere actualizar nuestro arranque, se indicará en sus
notas; no reemplaces el cargador de otro mod.

Para desinstalar, borrá solamente `nativePC/plugins/iceborne_damage_meter.dll`.
Podés conservar configuración e historial. No borres toda la carpeta nativePC.
Si también querés quitar el arranque, borrá nuestro `xinput9_1_0.dll` de la raíz
(o nuestro `xinput9_1_0.chain.dll` con Seamless) SOLO
si sigue siendo nuestro archivo y no fue reemplazado por otro mod. Si no estás
seguro, dejalo: sin el plugin no dibuja nada. Para identificarlo, Propiedades >
Detalles muestra el producto `Iceborne Damage Meter Bootstrap`; su SHA-256 debe
coincidir con bootstrapSha256 en el manifiesto de la versión que instalaste.

## Controles por defecto

- F8: mostrar u ocultar.
- Ctrl+F8: vista previa con datos ficticios.
- Ctrl+F6 o ≡: menú.
- F6: ajustes. F7: historial. Habilitan el puntero para hacer clic.
- Ctrl+F9: desbloquear/bloquear el arrastre.

En F6 podés ajustar la escala entre **40% y 175%** y la anchura entre **260 y 640**.
Con un panel angosto, nombres y cifras ocupan líneas separadas. El historial F7
identifica la misión y conserva su fecha; el detalle muestra también el objetivo.
Los registros anteriores siguen funcionando. Misiones no catalogadas o personalizadas
se identifican por su número, sin adivinar un nombre.

Para compartir, abrí el menú con Ctrl+F6 y hacé clic en **Enviar daño actual al chat**.
Envía un resumen compacto con el daño exacto de los participantes al
**canal seleccionado en el chat del juego**. El informe completo se puede copiar. Requiere daño real:
la vista previa no se comparte. No hay envíos automáticos; esperá 15 s entre pedidos.
No escribas mientras se completa; Escape o cambiar de ventana cancela lo pendiente.

La opción **Tecla de chat del juego** debe coincidir con tus controles: Insert
(predeterminada) o Enter. Si usás otra tecla, asigná una de esas dos en el juego.
Con ChatTransport=keyboard se usa entrada normal de texto pausada. El modo
experimental native usa una solicitud al chat del juego. Ninguno modifica el
portapapeles ni garantiza entrega; falta probar con un receptor online.

En **0.6.5**, el intento de envío se distingue de una entrega confirmada. Antes de
probarlo, entrá a una sesión online y comprobá que el chat normal abra con el menú
del medidor cerrado. El botón no crea una sesión ni habilita un chat que el propio
juego no permite abrir. La entrega de esta candidata sigue pendiente de validación.
Si el chat no está disponible, el menú ofrece **Copiar resumen de daño** para pegarlo
manualmente donde quieras. Solo esa acción explícita modifica el portapapeles.

Los INI antiguos se conservan al no reemplazarlos: `SettingsKey=121` usa F10;
`117` corresponde a F6, que evita el F10 fijo de Seamless.

## Si no aparece

Probá Ctrl+F8. No hace falta cambiar entre DirectX 11 y 12. Para reportar un
problema, indicá versión del mod, modo gráfico, cargador y otros mods. Adjuntá:

- `nativePC/plugins/iceborne_damage_meter.log`.
- `iceborne_damage_meter_bootstrap.log`, junto al EXE, si usás nuestro arranque.
- `nativePC/plugins/mhw-mod-loader.log`, si usás Seamless. Que ignore la carga
  directa es esperable; nuestro .chain.dll debe generar su propio log.

El manifiesto está en `IceborneDamageMeter/manifest.json` dentro del ZIP. La
lectura permanece desactivada si el ejecutable no coincide con el perfil
soportado; consultá `docs/COMPATIBILITY.md`.

## Alcance actual

Misiones normales: daño total de cada jugador, porcentaje del daño del equipo,
DPS observado, participación e historial. El porcentaje no representa la vida
del monstruo. Expediciones, Tierras Destino y asedios sin objetivo quedan fuera
del alcance. Las misiones con varios objetivos muestran totales de misión.

La candidata 0.6.3 pasó arranque, render de barras de vista previa y cierre dentro
del juego con Steam Overlay: DX12 con Seamless 1.0.2.0, con y sin Stracker, y DX11
con ambos cargadores, el 2026-11-11. La 0.6.2 fue retirada por un fallo al salir.
El daño en misión de esta candidata, la precisión multijugador y los FPS reales
siguen pendientes. Consultá `docs/VALIDATION.md`.
