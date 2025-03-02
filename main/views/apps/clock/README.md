# Clock View

## Descripción

Esta vista muestra un reloj digital y una animación de cuadrícula. El reloj muestra la hora en formato HH:MM:SS.  La cuadrícula se anima, iluminando una celda cada segundo. Al llegar al segundo 60 (completar un minuto), la cuadrícula se reinicia, mostrando todos los cuadrados en blanco.

## Interacción

* **Botón LEFT:** (Sin acción asignada)
* **Botón OK:** Cambiar color
* **Botón RIGHT:** Ir a Settings

## Estructura

La vista consta de:

*   Un `lv_label` para mostrar la hora.
*   Un `lv_obj` que actúa como contenedor de la cuadrícula.
*   Una matriz de `lv_obj` que representan las celdas de la cuadrícula.
*   Un `lv_timer` que actualiza la hora y la animación cada segundo.

## Consideraciones

*   La hora inicial es fija (12:00:00).  Se podría mejorar para obtener la hora de un RTC o servidor NTP.
* El color de la celda iluminada cambia de forma aleatoria cada vez.