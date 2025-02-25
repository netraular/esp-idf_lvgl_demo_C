#include "boot_view.h"
#include "controllers/screen_manager/screen_manager.h"
#include "esp_log.h"

static const char* TAG = "BOOT_VIEW";

BootView::BootView() : BaseView("Boot"), label(nullptr), timer(nullptr) {
    ESP_LOGI(TAG, "Creating Boot view");
    label = lv_label_create(screen);
    lv_label_set_text(label, "Booting...");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);

    timer = lv_timer_create([](lv_timer_t* t) {
        switch_screen("Clock");
    }, 2000, nullptr);
    lv_timer_set_repeat_count(timer, 1);
}

BootView::~BootView() {
    if (timer) {
        lv_timer_del(timer);
        timer = nullptr; // Añadido para consistencia
    }
    destroy(); // Llamada a destroy()
}