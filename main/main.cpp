#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "controllers/screen_manager/screen_manager.h"
#include "config.h"
#include "controllers/button_manager/button_manager.h"

static const char *TAG = "main";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Iniciando aplicación");

    // 1. Inicialización de hardware
    screen_t* screen = screen_init();
    if (!screen) {
        ESP_LOGE(TAG, "Fallo al inicializar pantalla");
        return;
    }

    button_manager_init();

    // 2. Gestión inicial de vistas
    switch_screen("Boot");
    ESP_LOGI(TAG, "Vista Boot mostrada");

    // 3. Bucle principal optimizado para 30 FPS
    ESP_LOGI(TAG, "Entrando en bucle principal");
    while (true) {
        const uint32_t t_start = esp_log_timestamp();

        lv_timer_handler();

        // Control de FPS: Asegurando 30 FPS
        const uint32_t elapsed = esp_log_timestamp() - t_start;
        const uint32_t delay_ms = elapsed < 33 ? 33 - elapsed : 1;  // Espera 33ms para mantener 30 FPS
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}
