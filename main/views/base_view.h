#ifndef BASE_VIEW_H
#define BASE_VIEW_H

#include "lvgl.h"
#include <string>

class BaseView {
protected:
    lv_obj_t* screen;
    std::string name;

public:
    BaseView(const std::string& view_name);
    virtual ~BaseView();

    virtual void register_button_handlers() = 0;
    virtual void unregister_button_handlers() = 0;
    virtual void destroy();

    lv_obj_t* get_screen() const { return screen; }
    std::string get_name() const { return name; }

    static lv_obj_t* create_base_screen();
};

#endif