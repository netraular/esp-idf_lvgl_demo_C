#include "lvgl.h"
#include "views/base_view.h"
#include "system_info_view.h"
#include "esp_log.h"
#include "controllers/button_manager/button_manager.h"
#include "controllers/screen_manager/screen_manager.h"

static const char *TAG = "SYSTEM_INFO_VIEW";
static lv_obj_t *label = nullptr;

static base_view_t system_info_view;

static void system_info_button3_handler() {
    ESP_LOGI(TAG, "Button 3 - Back to Settings");
    switch_screen("Settings");
}

static void system_info_view_register_button_handlers() {
    button_manager_register_view_handler(BUTTON_3, system_info_button3_handler);
}
static void system_info_view_unregister_button_handlers() {
    button_manager_unregister_view_handlers();
}

static void init_base_view(base_view_t* view, const char* name, void (*register_handlers)(void), void (*unregister_handlers)(void))
{
    if (view)
    {
        view->name = name;
        view->register_button_handlers = register_handlers;
        view->unregister_button_handlers = unregister_handlers;
        view->screen = create_base_view(name);
        lv_obj_set_user_data(view->screen, view);
    }
}

lv_obj_t *create_system_info_view() {
    ESP_LOGI(TAG, "Creating system info view");

    init_base_view(&system_info_view, "System Info", system_info_view_register_button_handlers, system_info_view_unregister_button_handlers);

    label = lv_label_create(system_info_view.screen);
    lv_label_set_text(label, "System Info");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);

    return system_info_view.screen;
}