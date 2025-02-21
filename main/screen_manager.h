// screen_manager.h
#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "lvgl.h"

// Estructura para contener los manejadores de la pantalla
typedef struct {
    esp_lcd_panel_io_handle_t io_handle;
    esp_lcd_panel_handle_t panel_handle;
    lv_disp_t *disp_handle;
} screen_t;

// Funciones
extern screen_t* screen_init();
extern void screen_deinit(screen_t* screen); // Destruir los handles y liberar recursos
extern void switch_screen(lv_obj_t* screen); // Cambiar la pantalla activa

#endif // SCREEN_MANAGER_H