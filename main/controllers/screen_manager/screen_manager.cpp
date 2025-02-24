#include "controllers/screen_manager/screen_manager.h"
#include "config.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h"
#include "esp_timer.h"
#include <vector>
#include <cstring>
#include "views/apps/clock/clock_view.h"
#include "views/system/boot_screen/boot_view.h"
#include "views/system/settings/settings_view.h"
#include "views/system/system_info/system_info_view.h"

static const char *TAG = "screen_manager";

static esp_timer_handle_t lv_tick_timer = nullptr;

static std::vector<base_view_t*> registered_views;
static base_view_t* current_view = nullptr;

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

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = TFT_RST,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        .bits_per_pixel = 16,
        .flags = {
            .reset_active_high = 0,
        },
        .vendor_config = nullptr,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(screen->io_handle, &panel_config, &screen->panel_handle));
    ESP_LOGI(TAG, "ST7789 panel initialized");

    esp_lcd_panel_reset(screen->panel_handle);
    ESP_ERROR_CHECK(esp_lcd_panel_init(screen->panel_handle));
    ESP_LOGI(TAG, "Panel initialized");

    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(screen->panel_handle, true));
    ESP_LOGI(TAG, "Color inversion set");
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(screen->panel_handle, true));
    ESP_LOGI(TAG, "Display turned on");

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

    screen_init_lvgl(screen);
    screen_init_lvgl_tick(screen);
    ESP_LOGI(TAG, "Screen initialization complete");
    return screen;
}

void screen_init_lvgl(screen_t* screen) {
    lv_init();

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
    screen_deinit_lvgl_tick();

    if (screen) {
        if (screen->panel_handle) {
            esp_lcd_panel_del(screen->panel_handle);
        }
        if (screen->io_handle) {
            esp_lcd_panel_io_del(screen->io_handle);
        }
        delete screen;
    }
}

void register_view(base_view_t* view) {
    if (view) {
        registered_views.push_back(view);
        ESP_LOGI(TAG, "View registered: %s", view->name);
    }
}

base_view_t* get_view(const char* name) {
    for (base_view_t* view : registered_views) {
        if (strcmp(view->name, name) == 0) {
            return view;
        }
    }
    ESP_LOGW(TAG, "View not found: %s", name);
    return nullptr;
}

lv_obj_t* create_view_by_name(const char* name) {
    if (strcmp(name, "Clock") == 0) {
        return create_clock_view();
    } else if (strcmp(name, "Boot") == 0) {
        return create_boot_view();
    } else if (strcmp(name, "Settings") == 0){
        return create_settings_view();
    } else if (strcmp(name, "System Info") == 0){
        return create_system_info_view();
    }
    ESP_LOGE(TAG, "Unknown view name: %s", name);
    return nullptr;
}

void switch_screen(const char* view_name) {
    ESP_LOGI(TAG, "Switching screen to: %s", view_name);

    if (current_view && current_view->unregister_button_handlers) {
        current_view->unregister_button_handlers();
        ESP_LOGI(TAG, "Unregistering button handlers for: %s", current_view->name);
    }

    if (current_view) {
       destroy_base_view(current_view->screen);
       ESP_LOGI(TAG, "Previous screen destroyed");
       current_view = nullptr;
    }

    lv_obj_t* new_screen = create_view_by_name(view_name);
    if (!new_screen) {
        ESP_LOGE(TAG, "Failed to create view: %s", view_name);
        return;
    }

    current_view = (base_view_t*)lv_obj_get_user_data(new_screen);

    if (current_view && current_view->register_button_handlers) {
        current_view->register_button_handlers();
        ESP_LOGI(TAG, "Registering button handlers for: %s", current_view->name);
    }

    lv_disp_load_scr(new_screen);
    ESP_LOGI(TAG, "New screen loaded: %s", view_name);
}

void destroy_current_view() {
    if (current_view && current_view->destroy) {
        current_view->destroy(); // Llama a la función específica de la vista
    }
}
