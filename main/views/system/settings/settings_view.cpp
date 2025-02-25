#include "settings_view.h"
#include "controllers/button_manager/button_manager.h"
#include "controllers/screen_manager/screen_manager.h"
#include "esp_log.h"

static const char* TAG = "SETTINGS_VIEW";

SettingsView::SettingsView() : BaseView("Settings"), label(nullptr) {
    ESP_LOGI(TAG, "Creating Settings view");
    label = lv_label_create(screen);
    lv_label_set_text(label, "Settings");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
}

SettingsView::~SettingsView() {
    destroy();
}

void SettingsView::register_button_handlers() {
    button_manager_register_view_handler(BUTTON_1, [](){
        switch_screen("Clock");
    });
    
    button_manager_register_view_handler(BUTTON_2, [](){
        switch_screen("System Info");
    });
}

void SettingsView::unregister_button_handlers() {
    button_manager_unregister_view_handlers();
}