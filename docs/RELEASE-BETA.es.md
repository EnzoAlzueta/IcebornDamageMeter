> Borrador histórico. La versión de desarrollo 0.6.14 retira el envío por chat;
> consultá la guía de instalación vigente para copiar resúmenes.

# Iceborne Damage Meter 0.6.10 — beta pública (borrador)

[English primary draft](RELEASE-BETA.md). La 0.6.10 estandariza los textos base
y documentación en inglés. Requiere una prueba breve en juego después de
instalarla; la validación de los últimos arreglos corresponde a la 0.6.9.

Medidor integrado de daño para Monster Hunter World: Iceborne en Windows x64.
Muestra el daño del equipo, porcentajes, DPS observado e historial de cacerías,
con tamaño y posición ajustables e idioma automático según el juego.

## Funciones incluidas

- Panel compacto y detalle de cacería; historial de las diez más recientes.
- Nombres de misiones y objetivos cuando existen en el catálogo. Las
  investigaciones con ID genérico 30000 se identifican como Investigación.
- Interfaz en 13 idiomas; traducciones de misiones no disponibles usan inglés.
- Copiar resumen y compartir daño con un clic. Los nombres se conservan cuando
  caben; grupos con entradas largas pueden ocupar varios mensajes.
- Iconos de armas corregidos, incluidos arco y ballesta ligera.
- Detección automática de DX11/DX12, configuración e historial persistentes.

## Instalación y actualización

Cerrar el juego y abrir su carpeta desde Steam.

- Sin cargador: ZIP normal, copiar nativePC y xinput9_1_0.dll.
- Con Stracker: copiar solamente nativePC del ZIP normal.
- Con Seamless: ZIP Seamless, copiar nativePC y xinput9_1_0.chain.dll;
  conservar el xinput9_1_0.dll original de Seamless. No sobrescribir una cadena
  perteneciente a otro mod.
- Al actualizar: conservar el INI propio y el historial.

El envío directo sin teclado se activa con ChatTransport=native bajo [Overlay]
en el INI. Los ZIP conservan keyboard como predeterminado. El directo fue probado
por el usuario en su instalación; mantener su condición experimental para otras
configuraciones. Requiere sesión online y confirmación del receptor.

Controles predeterminados: F8 mostrar/ocultar, Ctrl+F8 vista previa, F7 historial,
F6 ajustes, Ctrl+F6 menú y Ctrl+F9 mover. Un INI previo puede tener otras teclas.

## Límites conocidos

- No mide daño en expediciones ni Tierras Destino; muestra un aviso sin datos.
- Perfil de memoria 421810 y ejecutable compatible; Windows x64, SDR.
- DPS calculado desde la observación, incluyendo pausas y traslados; no es una
  comparación normalizada de habilidad. Investigaciones distintas no se comparan
  por compartir un ID genérico.
- No todos los idiomas, equipos y combinaciones de mods tienen pruebas reales.

## Evidencia de esta candidata

45 pruebas locales aprobadas, paquetes y hashes verificados. El usuario reportó
correctos los últimos ajustes de investigaciones, nombres completos y compartir
al volver a ciudad. Esto no certifica precisión universal ni toda la matriz de
compatibilidad. Publicar inicialmente como beta.

## Antes de publicar este borrador

- Completar instalación limpia/actualización/desinstalación siguiendo solo la guía.
- Revisar licencias y la distribución de los iconos originales extraídos del juego.
- Elegir canal de publicación y agregar capturas propias y enlaces de descarga.
- Si se publica un repositorio, comprobar su primera ejecución real de CI.
- Entregar ZIP normal, Seamless, fuente y checksums de la misma candidata.
- Publicar solo cuando el usuario indique destino y autorice la publicación.

Para reportar problemas: versión del medidor, DX11/DX12, cargador, misión/modo,
host/invitado, pasos y extracto pertinente del log sin datos personales.
