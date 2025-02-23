#ifndef CLOCK_VIEW_H
#define CLOCK_VIEW_H

#include "lvgl.h"

lv_obj_t* create_clock_view(lv_obj_t* parent);
void clock_view_register_button_handlers();
void clock_view_unregister_button_handlers();

#endif