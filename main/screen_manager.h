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
    lv_display_t *lvgl_disp;    // Nuevo: display de LVGL
    lv_color_t *lvgl_buf1;      // Nuevo: buffer 1
    lv_color_t *lvgl_buf2;      // Nuevo: buffer 2
} screen_t;

void screen_init_lvgl(screen_t* screen);

extern screen_t* screen_init();
extern void screen_deinit(screen_t* screen);
extern void switch_screen(lv_obj_t* screen);

#endif // SCREEN_MANAGER_H