#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "internal_components/screen_manager/screen_manager.h"
#include "views/apps/clock/clock_view.h"
#include "config.h"
#include "internal_components/button_manager/button_manager.h"
#include "esp_lcd_types.h"
#include "esp_check.h"

static const char *TAG = "main";

// Declarar la variable global screen
static screen_t* screen = nullptr;

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Starting application");

    // 1. Inicializar pantalla (esto ahora incluye la inicialización de LVGL)
    screen = screen_init();
    if (!screen) {
        ESP_LOGE(TAG, "Error al inicializar la pantalla.");
        return;
    }

    // 2. Inicializar gestor de botones
    button_manager_init();

    // 3. Crear vista inicial
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

    // 4. Limpieza
    clock_view_unregister_button_handlers();
    screen_deinit(screen);
    ESP_LOGI(TAG, "Application cleanup complete");
}