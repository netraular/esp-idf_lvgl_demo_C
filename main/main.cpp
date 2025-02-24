#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "controllers/screen_manager/screen_manager.h"
#include "views/apps/clock/clock_view.h"
#include "views/system/boot_screen/boot_view.h"
#include "views/system/settings/settings_view.h"
#include "views/system/system_info/system_info_view.h"
#include "config.h"
#include "controllers/button_manager/button_manager.h"
#include "esp_lcd_types.h"
#include "esp_check.h"


static const char *TAG = "main";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Starting application");

    static screen_t* screen = nullptr;

    // Inicializar pantalla
    screen = screen_init();
    if (!screen) {
        ESP_LOGE(TAG, "Error al inicializar la pantalla.");
        return;
    }

    // Inicializar gestor de botones
    button_manager_init();

    // Mostrar la vista de inicio (ya no es necesario crear las vistas aquí)
    switch_screen("Boot"); // Mostrar la vista de inicio
    ESP_LOGI(TAG, "Boot screen created and displayed");

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
}