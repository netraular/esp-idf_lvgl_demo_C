// screen_manager.h
#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "lvgl.h" // Include LVGL here

typedef struct {
    esp_lcd_panel_io_handle_t io_handle;
    esp_lcd_panel_handle_t panel_handle;
    // disp_handle is no longer needed, as we manage the LVGL display directly
} screen_t;

extern screen_t* screen_init();
extern void screen_deinit(screen_t* screen);
extern void switch_screen(lv_obj_t* screen);

#endif // SCREEN_MANAGER_H