# Iceborn Damage Meter 0.6.14 — actualización beta

- Permite ocultar el botón DM y desactivar la apertura al iniciar misión.
- Sigue registrando con el panel oculto; omite el dibujo cuando está oculto.
- Guarda cacerías en un archivo acumulativo opcional para análisis externo futuro.
  El historial del juego sigue mostrando diez.
- Retira el envío por chat y conserva Copiar resumen de daño.

Si el mod ya funciona, cerrar el juego y usar update-plugin-only: copiar nativePC
y reemplazar solamente nuestro plugin. No incluye loader, INI ni datos personales.
Para una instalación nueva, usar windows-x64 y su guía en inglés/español.
Conservar loader, ajustes, historial y archivo acumulativo al actualizar.

ArchiveHunts=1 en [Overlay] es el predeterminado; 0 y reinicio detiene el archivo.
F6 ofrece las dos opciones de visibilidad, activadas por defecto. Los atajos que
ya haya personalizado el usuario se conservan. El menú copia solo al portapapeles.

Pasaron cincuenta pruebas locales DX11/DX12 y de almacenamiento. La última sesión
del usuario en DX12 registró una cacería de cuatro jugadores sin errores en el
log; ambos archivos de cacerías se verificaron por formato y checksum. La evidencia
no certifica todos los controles visuales, reinicios, salida limpia, FPS ni todas
las configuraciones. Siguen fuera de soporte expediciones/Tierras Destino, HDR y
recuperación de pérdida del dispositivo. El log sigue sin límite ni rotación.

Release beta en GitHub:
https://github.com/EnzoAlzueta/IcebornDamageMeter/releases/tag/v0.6.14
La actualización de Nexus se gestiona por separado desde la cuenta del autor.
