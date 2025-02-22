// main.cpp
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

//  ----- CRUCIAL LVGL HEADER INCLUSION -----
#include "lvgl.h"         //  Include LVGL *before* ESP-IDF display headers

#include "screen_manager.h"
#include "views/color_grid_view.h"
#include "views/countdown_view.h"
#include "config.h"
#include "iot_button.h"
#include "esp_lcd_types.h" // Include for lv_color_t definition, if needed.
#include "esp_check.h"  //  VERY USEFUL for debugging ESP-IDF functions
#include "button_gpio.h"


static const char *TAG = "main";

typedef enum {
    VIEW_COLOR_GRID,
    VIEW_COUNTDOWN,
} app_view_t;

static app_view_t current_view = VIEW_COLOR_GRID;
static screen_t* screen; // From screen_manager

// LVGL display driver and draw buffer MUST be declared BEFORE disp_flush_cb
static lv_display_t * disp;
static lv_draw_buf_t draw_buf;
static lv_color_t *buf1;
static lv_color_t *buf2;
button_handle_t button4 = NULL;


// LVGL flush callback
// LVGL flush callback actualizado
static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    // Convertir el puntero de píxeles a lv_color_t*
    lv_color_t *color_p = reinterpret_cast<lv_color_t *>(px_map);
    
    if (screen && screen->panel_handle) {
        esp_lcd_panel_handle_t panel_handle = screen->panel_handle;
        int offsetx1 = area->x1;
        int offsetx2 = area->x2;
        int offsety1 = area->y1;
        int offsety2 = area->y2;
        
        // Usar el puntero convertido
        esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_p);
    }
    lv_display_flush_ready(disp);
}


// Button event handler
static void button_single_click_handler(void *arg,void *usr_data)
{
    ESP_LOGI(TAG, "Button event: SINGLE CLICK");
    if (current_view == VIEW_COLOR_GRID) {
        current_view = VIEW_COUNTDOWN;
        lv_obj_t* countdown_scr = create_countdown_view(nullptr);
        switch_screen(countdown_scr);
    } else if (current_view == VIEW_COUNTDOWN) {
        current_view = VIEW_COLOR_GRID;
        lv_obj_t* color_grid_scr = create_color_grid_view(nullptr);
        switch_screen(color_grid_scr);
    }
}

extern "C" void app_main(void) {
    // Initialize the screen
    screen = screen_init();
    if (!screen) {
        ESP_LOGE(TAG, "Error al inicializar la pantalla.");
        return;
    }

    // --- LVGL Initialization ---
    lv_init();

    // Buffer allocation (double buffering)
    buf1 = (lv_color_t*)heap_caps_malloc(SCREEN_WIDTH * 40 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(buf1);
    buf2 = (lv_color_t*)heap_caps_malloc(SCREEN_WIDTH * 40 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(buf2);

    //  Initialize the LVGL draw buffers
    lv_draw_buf_init(&draw_buf, SCREEN_WIDTH, 40, LV_COLOR_FORMAT_NATIVE, 0, buf1, SCREEN_WIDTH * 40 * sizeof(lv_color_t));

    if (draw_buf.data == NULL) { // Now we check for draw_buf.data
        ESP_LOGE(TAG, "Error al inicializar draw buffer.");
        return;
    }

    disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_buffers(disp, buf1, buf2, SCREEN_WIDTH * 40 * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL); // Pass buf1 and buf2 directly
    lv_display_set_flush_cb(disp, disp_flush_cb);

   // --- Button Configuration ---
   button_config_t btn_config = {
    .long_press_time=0,
    .short_press_time=0,
   };

   const button_gpio_config_t btn_gpio_cfg = {
        .gpio_num = 4,
        .active_level = 1, // Ajusta según la configuración de hardware
        .enable_power_save = false, // Deshabilita power save si no lo usas
        .disable_pull = false, // Usa resistencias internas si es necesario
    };


    button_handle_t button4 = NULL;
    esp_err_t ret = iot_button_new_gpio_device(&btn_config, &btn_gpio_cfg, &button4);
    ret = iot_button_register_cb(button4, BUTTON_SINGLE_CLICK, NULL, button_single_click_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE("BUTTON", "Error al registrar el callback del botón");
    }
    


    // Create and display the initial screen
    lv_obj_t* initial_screen = create_color_grid_view(nullptr);
    switch_screen(initial_screen);

    // --- Main Loop ---
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Cleanup
    screen_deinit(screen);
    if (button4) {
        iot_button_delete(button4);
    }
    free(buf1);
    free(buf2);
}