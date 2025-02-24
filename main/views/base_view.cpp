#include "base_view.h"
#include "esp_log.h"

static const char* TAG = "BASE_VIEW"; // Aunque no se use, lo dejo por consistencia.

lv_obj_t* create_base_view(const char* name) {
    lv_obj_t* screen = lv_obj_create(nullptr);
    lv_obj_set_size(screen, 240, 240);
    lv_obj_align(screen, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN); // Fondo blanco por defecto
    return screen;
}

void destroy_base_view(lv_obj_t* screen) {
    if(screen){
        lv_obj_del(screen); // LVGL handles the deletion
    }
}