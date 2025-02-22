// screen_manager.cpp
#include "screen_manager.h"
#include "config.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_check.h" // For error checking

static const char *TAG = "screen_manager";

typedef struct {
    lv_obj_t *current_screen;
} screen_internal_data_t;

static screen_internal_data_t screen_data;

screen_t* screen_init() {
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

    // 4. Inicialización y encendido del panel (CORRECT ORDER)
    esp_lcd_panel_reset(screen->panel_handle); // Hardware reset (pulse)
    ESP_ERROR_CHECK(esp_lcd_panel_init(screen->panel_handle));  // Initialize the controller

    // --- Orientation Settings ---
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(screen->panel_handle, true)); // Invert colors (often needed)


    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(screen->panel_handle, true)); // Turn on the display

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
    }

    screen_data.current_screen = nullptr;
    return screen;
}

void screen_deinit(screen_t* screen) {
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

void switch_screen(lv_obj_t* screen) {
    if(screen_data.current_screen) {
        lv_obj_del(screen_data.current_screen);
    }
    if(screen) {
        screen_data.current_screen = screen;
        lv_disp_load_scr(screen);
    }
}