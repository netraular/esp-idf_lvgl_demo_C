# Boot View

## Descripción

Esta vista es la pantalla de inicio. Muestra el texto "Booting..." durante 2 segundos y luego navega automáticamente a la vista del reloj ("Clock").

## Interacción

No hay interacción con el usuario en esta vista.

## Estructura

*   Un `lv_label` que muestra el texto "Booting...".
*   Un `lv_timer` que controla la duración de la vista.

## Consideraciones

* La duración de la vista es fija (2 segundos).