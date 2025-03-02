#include "system_info_view.h"
#include "controllers/button_manager/button_manager.h"
#include "controllers/screen_manager/screen_manager.h"
#include "esp_log.h"

static const char* TAG = "SYS_INFO_VIEW";

SystemInfoView::SystemInfoView() : BaseView("System Info"), label(nullptr) {
    ESP_LOGI(TAG, "Creating System Info view");
    label = lv_label_create(screen);
    lv_label_set_text(label, "System Info");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
}

SystemInfoView::~SystemInfoView() {
    destroy(); // Llamada a destroy.
}

void SystemInfoView::register_button_handlers() {
    button_manager_register_view_handler(BUTTON_OK, []() {
        switch_screen("Settings");
    });
}

void SystemInfoView::unregister_button_handlers() {
    button_manager_unregister_view_handlers();
}