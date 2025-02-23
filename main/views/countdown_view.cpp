#include "countdown_view.h"
#include "esp_log.h"
#include <atomic>  // Para manejo seguro en multi-thread

static const char *TAG = "COUNTDOWN";

// Variables globales para el contador y el timer
static std::atomic<int> count(10); // Contador atómico
static lv_timer_t *timer = nullptr; // Puntero al timer
static lv_obj_t *count_label = nullptr; // Label para mostrar el contador

// Timer callback para decrementar el contador
static void countdown_task(lv_timer_t *t) {
    count--;
    ESP_LOGI(TAG, "[DEBUG] Timer callback ejecutado. Nuevo valor: %d", count.load());
    
    if (count >= 0) {
        if (count_label) {
            lv_label_set_text_fmt(count_label, "%d", count.load());
            ESP_LOGD(TAG, "Label actualizado a %d", count.load());
            lv_refr_now(nullptr); // Forzar refresco de pantalla
            ESP_LOGD(TAG, "Pantalla refrescada");
        } else {
            ESP_LOGW(TAG, "Label no existe");
        }
    } else {
        ESP_LOGI(TAG, "Countdown terminado, deteniendo timer");
        lv_timer_del(t); // Detener el timer cuando llegue a 0
    }
}

lv_obj_t* create_countdown_view(lv_obj_t* parent) {
    ESP_LOGI(TAG, "Creando vista de countdown");

    // Eliminar timer anterior si existe
    if (timer) {
        ESP_LOGW(TAG, "Timer anterior eliminado: %p", timer);
        lv_timer_del(timer);
        timer = nullptr;
    }

    lv_obj_t* scr = lv_obj_create(parent);

    // Crear el label para mostrar el contador
    count_label = lv_label_create(scr);
    lv_label_set_text_fmt(count_label, "%d", count.load()); // Valor inicial
    lv_obj_set_style_text_font(count_label, &lv_font_montserrat_48, LV_PART_MAIN); // Fuente más grande
    lv_obj_center(count_label);

    // Reiniciar el contador solo si es la primera vez
    static bool first_init = true;
    if (first_init) {
        count = 10;
        first_init = false;
    }

    // Crear un nuevo timer
    timer = lv_timer_create(countdown_task, 1000, nullptr);
    ESP_LOGI(TAG, "Timer creado: %p", timer);

    return scr;
}