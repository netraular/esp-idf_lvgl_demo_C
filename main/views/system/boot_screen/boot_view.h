#ifndef BOOT_VIEW_H
#define BOOT_VIEW_H

#include "../../base_view.h"

class BootView : public BaseView {
private:
    lv_obj_t* label;
    lv_timer_t* timer;

public:
    BootView();
    virtual ~BootView();

    void register_button_handlers() override {}
    void unregister_button_handlers() override {}
};

#endif