#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "screen_manager.h"
#include "views/clock_view.h"
#include "config.h"
#include "button_manager.h"
#include "esp_lcd_types.h"
#include "esp_check.h"
#include "esp_timer.h"

static const char *TAG = "main";

// Declarar la variable global screen
static screen_t* screen = nullptr;

// Timer callback para el tick de LVGL
static void lv_tick_timer_cb(void *arg) {
    lv_tick_inc(1); // Actualizar el tick de LVGL cada 1ms
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Starting application");

    // 1. Inicializar pantalla (esto ahora incluye la inicialización de LVGL)
    screen = screen_init();
    if (!screen) {
        ESP_LOGE(TAG, "Error al inicializar la pantalla.");
        return;
    }

    // 2. Configurar el timer para el tick de LVGL
    esp_timer_handle_t lv_tick_timer;
    const esp_timer_create_args_t lv_tick_timer_args = {
        .callback = &lv_tick_timer_cb,  // Callback para el timer
        .arg = nullptr,                 // Argumento para el callback (no se usa)
        .dispatch_method = ESP_TIMER_TASK, // Método de despacho (usar la tarea de timer)
        .name = "lv_tick",              // Nombre del timer
        .skip_unhandled_events = true   // Ignorar eventos no manejados
    };

    ESP_ERROR_CHECK(esp_timer_create(&lv_tick_timer_args, &lv_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lv_tick_timer, 1000)); // Cada 1ms
    ESP_LOGI(TAG, "LVGL tick timer configured");

    // 3. Inicializar gestor de botones
    button_manager_init();

    // 4. Crear vista inicial
    lv_obj_t* initial_screen = create_clock_view(nullptr);
    switch_screen(initial_screen);
    ESP_LOGI(TAG, "Initial screen created and displayed");

    // --- Main Loop ---
    ESP_LOGI(TAG,"Entering main loop");
    while (1) {
        lv_timer_handler();
        
        // Log del loop principal cada 500ms
        static uint32_t last_log = 0;
        if (esp_log_timestamp() - last_log > 500) {
            ESP_LOGD(TAG, "[DEBUG] Loop activo. Tiempo: %ld", esp_log_timestamp());
            last_log = esp_log_timestamp();
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // 5. Limpieza
    clock_view_unregister_button_handlers();
    screen_deinit(screen);
    ESP_LOGI(TAG, "Application cleanup complete");
}