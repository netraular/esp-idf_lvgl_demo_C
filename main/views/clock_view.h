#ifndef CLOCK_VIEW_H
#define CLOCK_VIEW_H

#include "lvgl.h"
#include "iot_button.h"

// Declarar button4 como extern
extern button_handle_t button4;
extern button_handle_t button3;
extern button_handle_t button2;
extern button_handle_t button1;

lv_obj_t* create_clock_view(lv_obj_t* parent);
void clock_view_register_button_handlers();
void clock_view_unregister_button_handlers();

#endif