// main.cpp
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "screen_manager.h"
#include "ui_elements/color_grid.h" // Incluir la cabecera de la cuadrícula

static const char *TAG = "main";

extern "C" void app_main(void) {

    // 1. Inicializar la pantalla
    screen_t* screen = screen_init();
    if (!screen) {
        ESP_LOGE(TAG, "Error al inicializar la pantalla.");
        return;
    }

    // 2. Crear y mostrar la primera pantalla (ejemplo)
    lv_obj_t *scr = lv_obj_create(nullptr); // Crea una nueva pantalla
    lv_obj_t *color_grid = create_color_grid(scr);
    //lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN); // Fondo negro
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE); // Evita el scroll
    switch_screen(scr);


    // --- Bucle principal (solo llama a lv_timer_handler) ---
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    screen_deinit(screen); // Desinicializar la pantalla al finalizar
}