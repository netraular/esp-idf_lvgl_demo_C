#include "base_view.h"
#include "esp_log.h"

BaseView::BaseView(const std::string& view_name) : name(view_name) {
    screen = create_base_screen();
}

BaseView::~BaseView() {
    destroy();
}

void BaseView::destroy() {
    if(screen){
        lv_obj_del(screen);
        screen = nullptr;
    }
}

lv_obj_t* BaseView::create_base_screen() {
    lv_obj_t* screen = lv_obj_create(nullptr);
    lv_obj_set_size(screen, 240, 240);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN);
    return screen;
}