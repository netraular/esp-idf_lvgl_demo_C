#include "screen_manager.h"
#include "config.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_timer.h"

static const char *TAG = "screen_manager";

// Variable para el timer del tick de LVGL
static esp_timer_handle_t lv_tick_timer = nullptr;

typedef struct {
    lv_obj_t *current_screen;
} screen_internal_data_t;

static screen_internal_data_t screen_data;

// Función para inicializar el timer de LVGL
static void screen_init_lvgl_tick(screen_t* screen) {
    const esp_timer_create_args_t lv_tick_timer_args = {
        .callback = [](void* arg) {
            lv_tick_inc(1);
        },
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lv_tick",
        .skip_unhandled_events = true
    };

    ESP_ERROR_CHECK(esp_timer_create(&lv_tick_timer_args, &lv_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lv_tick_timer, 1000));
    ESP_LOGI(TAG, "LVGL tick timer initialized");
}

// Función para detener el timer
static void screen_deinit_lvgl_tick() {
    if (lv_tick_timer) {
        esp_timer_stop(lv_tick_timer);
        esp_timer_delete(lv_tick_timer);
        lv_tick_timer = nullptr;
        ESP_LOGI(TAG, "LVGL tick timer deinitialized");
    }
}

screen_t* screen_init() {
    ESP_LOGI(TAG, "Initializing screen");
    screen_t* screen = new screen_t;
    if (!screen) {
        ESP_LOGE(TAG, "No se pudo reservar memoria para la pantalla.");
        return nullptr;
    }
    screen->io_handle = nullptr;
    screen->panel_handle = nullptr;

    // 1. Inicialización del bus SPI
    spi_bus_config_t buscfg = {
        .mosi_io_num = TFT_MOSI,
        .miso_io_num = TFT_MISO,
        .sclk_io_num = TFT_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .data_io_default_level = false,
        .max_transfer_sz = SCREEN_WIDTH * 40 * sizeof(lv_color_t),
        .flags = 0,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
        .intr_flags = 0,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "SPI bus initialized successfully");

    // 2. Configuración de la interfaz SPI para la pantalla
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = TFT_CS,
        .dc_gpio_num = TFT_DC,
        .spi_mode = 0,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .on_color_trans_done = nullptr,
        .user_ctx = nullptr,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .flags = {
            .dc_high_on_cmd = 0,
            .dc_low_on_data = 0,
            .dc_low_on_param = 0,
            .octal_mode = 0,
            .quad_mode = 0,
            .sio_mode = 0,
            .lsb_first = 0,
            .cs_high_active = 0,
        },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &screen->io_handle));
    ESP_LOGI(TAG, "Panel IO configured");

    // 3. Configuración del panel ST7789 (and initialization)
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = TFT_RST,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        .bits_per_pixel = 16,
        .flags = {
            .reset_active_high = 0, //  Active LOW reset is common
        },
        .vendor_config = nullptr,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(screen->io_handle, &panel_config, &screen->panel_handle));
    ESP_LOGI(TAG, "ST7789 panel initialized");

    // 4. Inicialización y encendido del panel (CORRECT ORDER)
    esp_lcd_panel_reset(screen->panel_handle); // Hardware reset (pulse)
    ESP_ERROR_CHECK(esp_lcd_panel_init(screen->panel_handle));  // Initialize the controller
    ESP_LOGI(TAG, "Panel initialized");

    // --- Orientation Settings ---
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(screen->panel_handle, true)); // Invert colors (often needed)
    ESP_LOGI(TAG, "Color inversion set");

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(screen->panel_handle, true)); // Turn on the display
    ESP_LOGI(TAG, "Display turned on");

     // 5. Control del backlight
    if (TFT_BL >= 0) {
       gpio_config_t bk_gpio_config = {
            .pin_bit_mask = 1ULL << TFT_BL,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
        gpio_set_level(TFT_BL, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
        ESP_LOGI(TAG, "Backlight configured and turned on");
    }

    screen_data.current_screen = nullptr;

    screen_init_lvgl(screen); 
    screen_init_lvgl_tick(screen);  // Inicializar el timer de LVGL
    ESP_LOGI(TAG, "Screen initialization complete");
    return screen;
}

void screen_init_lvgl(screen_t* screen) {
    lv_init();
    
    // Configurar buffers
    screen->lvgl_buf1 = (lv_color_t*)heap_caps_malloc(SCREEN_WIDTH * 40 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    screen->lvgl_buf2 = (lv_color_t*)heap_caps_malloc(SCREEN_WIDTH * 40 * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    assert(screen->lvgl_buf1 && screen->lvgl_buf2);

    lv_draw_buf_t draw_buf;
    lv_draw_buf_init(&draw_buf, SCREEN_WIDTH, 40, LV_COLOR_FORMAT_NATIVE, 0, screen->lvgl_buf1, SCREEN_WIDTH * 40 * sizeof(lv_color_t));

    screen->lvgl_disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_buffers(screen->lvgl_disp, screen->lvgl_buf1, screen->lvgl_buf2, SCREEN_WIDTH * 40 * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(screen->lvgl_disp, [](lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
        screen_t* s = (screen_t*)lv_display_get_user_data(disp);
        esp_lcd_panel_draw_bitmap(s->panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (lv_color_t*)px_map);
        lv_display_flush_ready(disp);
    });
    
    lv_display_set_user_data(screen->lvgl_disp, screen);
}

void screen_deinit(screen_t* screen) {
    ESP_LOGI(TAG, "Deinitializing screen");
    screen_deinit_lvgl_tick();  // Detener el timer de LVGL

    if (screen) {
        if (screen->panel_handle) {
            esp_lcd_panel_del(screen->panel_handle);
            ESP_LOGI(TAG, "Panel handle deleted");
        }
        if (screen->io_handle) {
            esp_lcd_panel_io_del(screen->io_handle);
            ESP_LOGI(TAG, "IO handle deleted");
        }
        delete screen;
        ESP_LOGI(TAG, "Screen structure freed");
    }
}

void switch_screen(lv_obj_t* new_screen) {
    ESP_LOGI(TAG, "Switching screen. Current: %p, New: %p", screen_data.current_screen, new_screen);
    
    // Limpiar vista actual antes de cambiar
    if (screen_data.current_screen) {
        // Enviar evento de eliminación a la pantalla actual
        lv_obj_send_event(screen_data.current_screen, LV_EVENT_DELETE, NULL);
        
        // Eliminar la pantalla actual
        lv_obj_del(screen_data.current_screen);
        ESP_LOGI(TAG, "Old screen deleted: %p", screen_data.current_screen);
    }
    
    // Asignar la nueva pantalla
    if (new_screen) {
        screen_data.current_screen = new_screen;
        ESP_LOGI(TAG, "Loading new screen: %p", new_screen);
        
        // Cargar la nueva pantalla en el display
        lv_disp_load_scr(new_screen);
    } else {
        screen_data.current_screen = nullptr;
        ESP_LOGI(TAG, "No new screen provided, current screen set to nullptr");
    }
}