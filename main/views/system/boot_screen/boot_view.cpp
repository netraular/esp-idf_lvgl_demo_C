#include "lvgl.h"
#include "views/base_view.h"
#include "boot_view.h"
#include "esp_log.h"
#include "controllers/screen_manager/screen_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BOOT_VIEW";
static lv_obj_t *label = nullptr;

static base_view_t boot_view;

static void boot_screen_timer_callback(lv_timer_t *timer) {
    ESP_LOGI(TAG, "Boot screen timer expired, switching to clock view");
    switch_screen("Clock");
}

static void  register_button_handlers(void){}
static void  unregister_button_handlers(void){}

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



lv_obj_t* create_boot_view() {
    ESP_LOGI(TAG, "Creating boot view");
    init_base_view(&boot_view, "Boot", register_button_handlers, unregister_button_handlers);

    label = lv_label_create(boot_view.screen);
    lv_label_set_text(label, "Booting...");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);

    lv_timer_t * timer = lv_timer_create(boot_screen_timer_callback, 2000, boot_view.screen);
    lv_timer_set_repeat_count(timer, 1);

    return boot_view.screen;
}