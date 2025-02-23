#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "screen_manager.h"
#include "views/color_grid_view.h"
#include "views/countdown_view.h"
#include "views/clock_view.h"
#include "config.h"
#include "iot_button.h"
#include "esp_lcd_types.h"
#include "esp_check.h"
#include "button_gpio.h"
#include "esp_timer.h"

static const char *TAG = "main";

typedef enum {
    VIEW_CLOCK,
    VIEW_COLOR_GRID,
    VIEW_COUNTDOWN,
} app_view_t;

static app_view_t current_view = VIEW_CLOCK;
static screen_t* screen; // From screen_manager

// LVGL display driver and draw buffer MUST be declared BEFORE disp_flush_cb
static lv_display_t * disp;
static lv_draw_buf_t draw_buf;
static lv_color_t *buf1;
static lv_color_t *buf2;
button_handle_t button4 = NULL;

// LVGL flush callback
static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    lv_color_t *color_p = reinterpret_cast<lv_color_t *>(px_map);
    
    if (screen && screen->panel_handle) {
        esp_lcd_panel_handle_t panel_handle = screen->panel_handle;
        
        // Usar %hd y casting a short
        ESP_LOGD(TAG, "[DEBUG] Área refrescada: X(%hd-%hd), Y(%hd-%hd)", 
                (short)area->x1, (short)area->x2, (short)area->y1, (short)area->y2);
        
        esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);
    }
    lv_display_flush_ready(disp);
}

// Button event handler
static void button_single_click_handler(void *arg,void *usr_data) {
    ESP_LOGI(TAG, "[DEBUG] Botón presionado! Estado actual: %d", current_view);

	if (current_view == VIEW_CLOCK) {
		ESP_LOGI(TAG, "Switching to COLOR GRID view");
		current_view = VIEW_COLOR_GRID;
		lv_obj_t* color_grid_scr = create_color_grid_view(nullptr);
		switch_screen(color_grid_scr);
	} else if (current_view == VIEW_COLOR_GRID) {
		ESP_LOGI(TAG, "Switching to CLOCK view");
		current_view = VIEW_CLOCK;
		lv_obj_t* clock_scr = create_clock_view(nullptr);
		switch_screen(clock_scr);
	} else if (current_view == VIEW_COUNTDOWN) {
		//Si se desea hacer algo diferente al pulsar el botón
        // desde la vista de countdown
	}

	ESP_LOGI(TAG, "New current view: %d", current_view);
}

// Timer callback para el tick de LVGL
static void lv_tick_timer_cb(void *arg) {
    lv_tick_inc(1); // Actualizar el tick de LVGL cada 1ms
}

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Starting application");

    // Initialize the screen
    screen = screen_init();
    if (!screen) {
        ESP_LOGE(TAG, "Error al inicializar la pantalla.");
        return;
    }

    // --- LVGL Initialization ---
    lv_init();
    ESP_LOGI(TAG, "LVGL initialized");

    // Configurar el timer para el tick de LVGL
    esp_timer_handle_t lv_tick_timer;
    const esp_timer_create_args_t lv_tick_timer_args = {
        .callback = &lv_tick_timer_cb,  // Callback para el timer
        .arg = nullptr,                 // Argumento para el callback (no se usa)
        .dispatch_method = ESP_TIMER_TASK, // Método de despacho (usar la tarea de timer)
        .name = "lv_tick",              // Nombre del timer
        .skip_unhandled_events = true   // Ignorar eventos no manejados
    };

    ESP_ERROR_CHECK(esp_timer_create(&lv_tick_timer_args, &lv_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lv_tick_timer, 1000)); // Cada 1ms
    ESP_LOGI(TAG, "LVGL tick timer configured");

    // Buffer allocation (double buffering)
    buf1 = (lv_color_t*)heap_caps_malloc(SCREEN_WIDTH * 40 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(buf1);
    buf2 = (lv_color_t*)heap_caps_malloc(SCREEN_WIDTH * 40 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(buf2);
    ESP_LOGI(TAG, "Display buffers allocated");

    // Initialize the LVGL draw buffers
    lv_draw_buf_init(&draw_buf, SCREEN_WIDTH, 40, LV_COLOR_FORMAT_NATIVE, 0, buf1, SCREEN_WIDTH * 40 * sizeof(lv_color_t));

    if (draw_buf.data == NULL) {
        ESP_LOGE(TAG, "Error al inicializar draw buffer.");
        return;
    }

    disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_buffers(disp, buf1, buf2, SCREEN_WIDTH * 40 * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, disp_flush_cb);
    ESP_LOGI(TAG, "LVGL display created and configured");

    // --- Button Configuration ---
    button_config_t btn_config = {
        .long_press_time = 0,
        .short_press_time = 0,
    };

    const button_gpio_config_t btn_gpio_cfg = {
        .gpio_num = 4,
        .active_level = 1,
        .enable_power_save = false,
        .disable_pull = false,
    };

    button_handle_t button4 = NULL;
    esp_err_t ret = iot_button_new_gpio_device(&btn_config, &btn_gpio_cfg, &button4);
    ret = iot_button_register_cb(button4, BUTTON_SINGLE_CLICK, NULL, button_single_click_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE("BUTTON", "Error al registrar el callback del botón");
    }
    ESP_LOGI(TAG, "Button configured and callback registered");

    // Create and display the initial screen
    lv_obj_t* initial_screen = create_clock_view(nullptr);
    switch_screen(initial_screen);
    ESP_LOGI(TAG, "Initial screen created and displayed");

    // --- Main Loop ---
    #define ANSI_COLOR_MAGENTA "\x1b[35m" // Lila/Magenta
    #define ANSI_COLOR_RESET   "\x1b[0m"  // Resetear a los colores por defecto
    ESP_LOGI(TAG, ANSI_COLOR_MAGENTA "Entering main loop" ANSI_COLOR_RESET);
    ESP_LOGI(TAG, "Entering main loop");
    while (1) {
        lv_timer_handler();
        
        // Log del loop principal cada 500ms
        static uint32_t last_log = 0;
        if (esp_log_timestamp() - last_log > 500) {
            ESP_LOGD(TAG, "[DEBUG] Loop activo. Tiempo: %ld", esp_log_timestamp());
            last_log = esp_log_timestamp();
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Cleanup
    screen_deinit(screen);
    if (button4) {
        iot_button_delete(button4);
    }
    free(buf1);
    free(buf2);
    ESP_LOGI(TAG, "Application cleanup complete");
}