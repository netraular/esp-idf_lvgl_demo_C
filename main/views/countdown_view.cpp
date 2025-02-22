// countdown_view.cpp
#include "countdown_view.h"
#include "lvgl.h"

static lv_obj_t *count_label; // Para actualizar el texto
static int count = 10;       // El valor inicial del contador

// Tarea para actualizar el contador
static void countdown_task(lv_timer_t *timer) {
    count--;
    if (count >= 0) {
        lv_label_set_text_fmt(count_label, "%d", count);
    } else {
        lv_timer_del(timer); // Detener el timer cuando llegue a 0
    }
}


lv_obj_t* create_countdown_view(lv_obj_t* parent) {
    lv_obj_t* scr = lv_obj_create(parent);

    // Crear la etiqueta para mostrar el contador
    count_label = lv_label_create(scr);
    lv_label_set_text(count_label, "10"); // Valor inicial
    lv_obj_set_style_text_font(count_label, &lv_font_montserrat_48, LV_PART_MAIN); // Fuente más grande
    lv_obj_center(count_label);

    // Crear un timer para actualizar el contador cada segundo
    lv_timer_create(countdown_task, 1000, nullptr);

     return scr;
}