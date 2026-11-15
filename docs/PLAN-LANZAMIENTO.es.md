# Plan para terminar y lanzar Iceborne Damage Meter

[English primary plan](RELEASE-PLAN.md). Candidata preparada: 0.6.10, con inglés
como idioma base; instalación del usuario: 0.6.9. Conservar la validación histórica
sin atribuir a la nueva candidata pruebas reales que aún no se hicieron.

Actualización: 0.6.9 instalada. El usuario confirmó que los últimos arreglos de
investigaciones, nombres y envío desde ciudad funcionan. Mantener las funciones
congeladas y pasar a preparación de beta; expediciones quedan pospuestas por
decisión del usuario. La matriz completa de abajo no se considera aprobada por
esa confirmación. Hay borrador público en RELEASE-BETA.es.md.

Estado de partida: 0.6.7 instalada, 45 pruebas locales aprobadas. Todavía no es
una versión estable certificada. Este plan no inicia una publicación ni cambia
la instalación del usuario.

## 1. Cerrar el alcance de la primera publicación

Propuesta: primera beta pública centrada en misiones, arena y eventos.
Mantener interfaz/historial multilingües, ajustes de tamaño e iconos, y resumen
manual de daño. No agregar nuevas funciones mientras se validan estas.

La medición del daño real del grupo en expediciones y Tierras Destino queda
fuera del alcance propuesto: requiere otra fuente de datos. La beta debe mostrar
un aviso claro y nunca reutilizar cifras de una misión anterior. No anunciar
soporte de daño en expedición por haber corregido ese aviso.

El envío nativo sigue experimental y desactivado por defecto hasta comprobar
recepción real. Si el envío por teclado tampoco resulta fiable, publicar con
Copiar resumen como alternativa principal y describir el envío como experimental.

Salida: lista de funciones incluidas y limitaciones coherente en interfaz,
guía, descripción pública y notas de versión.

## 2. Validar la 0.6.7 jugando y corregir los fallos encontrados

El usuario y su amigo realizan las acciones de juego; el desarrollo registra
versiones, escenario, capturas, logs y resultados. Las pruebas locales no
sustituyen esta comprobación.

| Escenario | Resultado necesario |
|---|---|
| Misión normal, solo y con amigo | Contadores nuevos, jugadores correctos y totales coherentes |
| Arena y evento | Daño, título, finalización e historial correctos |
| Misión → expedición → misión | Sin cifras heredadas; aviso en expedición; nueva misión desde cero |
| Expedición → pueblo y cambio de mapa | Sin datos anteriores presentados como daño actual |
| Éxito, fallo, retirada y repetición de misión | Un registro por cacería, desenlace y reinicio correctos |
| Entrada/salida/reconexión del amigo | Sin duplicar jugadores ni perder daño ya registrado |
| Compartir por teclado | Un clic, mensaje completo y recepción confirmada por el amigo |
| Compartir nativo, prueba separada | Sin apertura/teclas, borrador protegido y recepción confirmada |
| Reiniciar y cerrar el juego | Sin cierres anormales; ajustes e historial conservados |

Comprobar precisión con golpes controlados y registros de ambos participantes,
documentando qué daños incluye el contador. Una comparación con otro lector del
mismo contador no constituye por sí sola una verificación independiente.

Salida: resultados reales de 0.6.7 o su corrección, sin fallos bloqueantes.
Un resultado desconocido queda registrado como pendiente, no aprobado.

## 3. Revisar idiomas, presentación y compatibilidad

- Probar detección real en español e inglés; español ya fue detectado mediante
  Steam en 0.6.6. Revisar portugués y muestras CJK/árabe con textos largos.
- Revisar nombres de misiones, avisos y etiquetas; pedir revisión a hablantes
  cuando sea posible. Distinguir cobertura de traducción de calidad lingüística.
- Revisar panel mínimo/angosto, nombres largos, historial y avisos sin cortes.
- Repetir arranque, cacería y cierre en DX11 y DX12 con la candidata final.
- Verificar las rutas sin cargador, con Stracker y con Seamless en instalaciones
  separadas, preservando los cargadores existentes.
- Si no se prueba un escenario, declarar su estado y limitar las afirmaciones.
  Ampliar a cuatro jugadores antes de certificar ese escenario como probado.

Salida: matriz de compatibilidad con resultados, versiones y límites explícitos.

## 4. Preparar una candidata final reproducible

- Corregir los problemas de los pasos anteriores y congelar las funciones.
- Ejecutar compilación, suite completa, auditoría de traducciones y verificación
  de paquetes sobre los mismos binarios que se entregarán.
- Probar actualización desde la versión instalada sin reemplazar INI/historial.
- Probar instalación limpia y desinstalación con alguien que siga solo la guía.
- Unificar documentación: algunos apartados conservan estados históricos de
  versiones anteriores. Separar estado vigente y antecedentes.
- Revisar licencias, atribuciones y la distribución de los iconos extraídos del
  juego; preparar iconos propios si no se acredita autorización para incluirlos.
- Preparar ZIP normal, ZIP Seamless, fuente y checksums; excluir herramientas de
  investigación, respaldos, datos personales y fixtures de los paquetes runtime.
- Crear el repositorio y ejecutar realmente el workflow preparado de CI antes
  de anunciar compilaciones verificadas por ese servicio.

Salida: candidata identificada por versión y hashes, guías comprobadas y respaldo.

## 5. Publicar la beta

Preparar una página con descripción, capturas propias, requisitos, instalación
normal/Stracker/Seamless, controles, actualización, desinstalación y límites.
Adjuntar los paquetes verificados y una plantilla de reportes: versión, API
gráfica, cargador, tipo de cacería, host/invitado, pasos y log pertinente.

El usuario elige el canal de distribución y autoriza la publicación concreta.
No anunciar estabilidad, precisión universal ni compatibilidad sin pruebas.

Salida: beta pública descargable y un canal claro para reportar problemas.

## 6. Pasar a estable

Recoger pruebas en equipos e instalaciones ajenas y resolver los problemas
bloqueantes de arranque/cierre, exactitud, pérdida de historial o daño heredado.
Revalidar los cambios en los escenarios afectados y publicar notas de versión.
Mantener expediciones sin medición y envío nativo experimental fuera de las
promesas de estabilidad mientras continúen sin evidencia suficiente.

La versión estable se decide por estos resultados, no por una fecha ni por
alcanzar un número determinado de versión.

## Orden inmediato

Primero: resultado de las pruebas del usuario sobre 0.6.7. Después: corregir lo
observado, probar el envío nativo por separado y cerrar la matriz de escenarios.
La investigación de daño de expediciones puede continuar después de la primera
beta; incluirla antes implica ampliar el alcance y volver a validar el lector.
