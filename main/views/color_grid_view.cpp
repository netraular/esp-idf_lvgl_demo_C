#include "color_grid_view.h"
#include "esp_log.h"
#include <atomic>  // Para manejo seguro en multi-thread

static const char *TAG = "COLOR_GRID";

// Definimos los colores como macros (podrían estar en un archivo de configuración común)
constexpr uint32_t MY_COLOR_RED = 0xFF0000;
constexpr uint32_t MY_COLOR_GREEN = 0x00FF00;
constexpr uint32_t MY_COLOR_BLUE = 0x0000FF;
constexpr uint32_t MY_COLOR_YELLOW = 0xFFFF00;

struct ColorInfo {
    uint32_t color;
    const char* name;
};

static const struct ColorInfo palette_colors[] = {
    {MY_COLOR_RED, "Red"},
    {MY_COLOR_GREEN, "Green"},
    {MY_COLOR_BLUE, "Blue"},
    {MY_COLOR_YELLOW, "Yellow"},
};

// Variables globales para el contador y el timer
static std::atomic<int> current_number(0); // Contador atómico
static lv_timer_t *timer = nullptr;        // Puntero al timer
static lv_obj_t *number_label = nullptr;   // Label para mostrar el número

// Timer callback para incrementar el número
static void increment_number_task(lv_timer_t *t) {
    current_number++;
    ESP_LOGI(TAG, "[DEBUG] Timer callback ejecutado. Nuevo valor: %d", current_number.load()); // Log detallado
    
    // Actualizar solo si el label existe
    if (number_label) {
        lv_label_set_text_fmt(number_label, "%d", current_number.load());
        ESP_LOGD(TAG, "Label actualizado a %d", current_number.load());
        lv_refr_now(nullptr); // Forzar refresco de pantalla
        ESP_LOGD(TAG, "Pantalla refrescada");
    } else {
        ESP_LOGW(TAG, "Label no existe");
    }
}

lv_obj_t* create_color_grid_view(lv_obj_t* parent) {
    ESP_LOGI(TAG, "Creando vista de color grid");

    // Eliminar timer anterior si existe
    if (timer) {
        ESP_LOGW(TAG, "Timer anterior eliminado: %p", timer);
        lv_timer_del(timer);
        timer = nullptr;
    }

    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 240, 240);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);

    for (size_t i = 0; i < 4; i++) {
        lv_obj_t* cell = lv_obj_create(cont);
        lv_obj_set_size(cell, 120, 120);
        int row = i / 2;
        int col = i % 2;
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
        lv_obj_set_style_bg_color(cell, lv_color_hex(palette_colors[i].color), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, LV_PART_MAIN);

        lv_obj_t* label = lv_label_create(cell);
        lv_label_set_text(label, palette_colors[i].name);
        lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
        lv_obj_center(label);
    }

    // Crear el label para mostrar el número
    number_label = lv_label_create(cont);
    lv_label_set_text_fmt(number_label, "%d", current_number.load()); // Valor inicial
    lv_obj_set_style_text_font(number_label, &lv_font_montserrat_48, LV_PART_MAIN); // Fuente más grande
    lv_obj_set_style_text_color(number_label, lv_color_black(), LV_PART_MAIN); // Asegurar que el texto sea visible

    // Posicionar el label en el centro
    lv_obj_set_grid_cell(number_label, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 2);

    // Reiniciar el contador solo si es la primera vez
    static bool first_init = true;
    if (first_init) {
        current_number = 0;
        first_init = false;
    }

    // Crear un nuevo timer
    timer = lv_timer_create(increment_number_task, 1000, nullptr);
    ESP_LOGI(TAG, "Timer creado: %p", timer);

    return cont;
}