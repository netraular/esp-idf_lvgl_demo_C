#include "lvgl.h"
#include "views/base_view.h"
#include "settings_view.h"
#include "esp_log.h"
#include "controllers/button_manager/button_manager.h"
#include "controllers/screen_manager/screen_manager.h"

static const char *TAG = "SETTINGS_VIEW";
static lv_obj_t *label = nullptr;
static base_view_t settings_view;

static void settings_button1_handler() {
    ESP_LOGI(TAG, "Button 1 - Back to Clock");
    switch_screen("Clock");
}

static void settings_button2_handler() {
    ESP_LOGI(TAG, "Button 2 - Go to System Info");
    switch_screen("System Info");
}

static void settings_view_register_button_handlers() {
    button_manager_register_view_handler(BUTTON_1, settings_button1_handler);
    button_manager_register_view_handler(BUTTON_2, settings_button2_handler);
}

static void settings_view_unregister_button_handlers() {
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


lv_obj_t *create_settings_view() {
    ESP_LOGI(TAG, "Creating settings view");
    init_base_view(&settings_view, "Settings", settings_view_register_button_handlers, settings_view_unregister_button_handlers);

    label = lv_label_create(settings_view.screen);
    lv_label_set_text(label, "Settings");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);

    return settings_view.screen;
}