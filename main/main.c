#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "config.h"

static const char *TAG = "main";

static lv_disp_t *disp_handle = NULL;

void view_colors_task(void *pvParameters) {
    lv_obj_t *scr = lv_disp_get_scr_act(disp_handle);

    // Usamos un array de lv_color_t directamente.
    lv_color_t palette_colors[] = {
        lv_palette_main(LV_PALETTE_RED),
        lv_palette_main(LV_PALETTE_PINK),
        lv_palette_main(LV_PALETTE_PURPLE),
        lv_palette_main(LV_PALETTE_DEEP_PURPLE),
        lv_palette_main(LV_PALETTE_INDIGO),
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_LIGHT_BLUE),
        lv_palette_main(LV_PALETTE_CYAN),
        lv_palette_main(LV_PALETTE_TEAL),
        lv_palette_main(LV_PALETTE_GREEN),
        lv_palette_main(LV_PALETTE_LIGHT_GREEN),
        lv_palette_main(LV_PALETTE_LIME),
        lv_palette_main(LV_PALETTE_YELLOW),
        lv_palette_main(LV_PALETTE_AMBER),
        lv_palette_main(LV_PALETTE_ORANGE),
        lv_palette_main(LV_PALETTE_DEEP_ORANGE),
        lv_palette_main(LV_PALETTE_BROWN),
        lv_palette_main(LV_PALETTE_BLUE_GREY),
        lv_palette_main(LV_PALETTE_GREY),
    };

    int color_index = 0;
    int palette_len = sizeof(palette_colors) / sizeof(palette_colors[0]);

    while (1) {
        if (color_index == 0) {
            lv_obj_set_style_bg_color(scr, lv_color_white(), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
            vTaskDelay(pdMS_TO_TICKS(500));
            lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        lv_obj_set_style_bg_color(scr, palette_colors[color_index], LV_PART_MAIN);
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

        color_index = (color_index + 1) % palette_len;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    spi_bus_config_t buscfg = {
        .miso_io_num = TFT_MISO,
        .mosi_io_num = TFT_MOSI,
        .sclk_io_num = TFT_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SCREEN_WIDTH * 40 * sizeof(lv_color_t),
        .flags = 0,
        .intr_flags = 0,
		.data4_io_num = -1,
		.data5_io_num = -1,
		.data6_io_num = -1,
		.data7_io_num = -1,
		.data_io_default_level = 0,
		.isr_cpu_id = 0
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = TFT_DC,
        .cs_gpio_num = TFT_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL, // Usar NULL en C
        .user_ctx = NULL,
        .flags = {}
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = TFT_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
        .flags = {},  //Inicializar flags
        .vendor_config = NULL

    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));


    if (TFT_BL >= 0) {
        gpio_config_t bk_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << TFT_BL,
            .pull_down_en = GPIO_PULLDOWN_DISABLE, // Asegúrate de deshabilitar pull-down
            .pull_up_en = GPIO_PULLUP_DISABLE,   // y pull-up si no los necesitas
            .intr_type = GPIO_INTR_DISABLE //Sin interrupcion
        };
        ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
        gpio_set_level(TFT_BL, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
    }

	const lvgl_port_display_cfg_t disp_cfg = {
	    .io_handle = io_handle,
	    .panel_handle = panel_handle,
	    .buffer_size = SCREEN_WIDTH * 40 * sizeof(lv_color_t),
	    .double_buffer = true,
	    .hres = SCREEN_WIDTH,
	    .vres = SCREEN_HEIGHT,
	    .monochrome = false,
	    .rotation = {
	        .swap_xy = false,
	        .mirror_x = false,
	        .mirror_y = false,
	    },
    .flags = {} //Inicializa flags
	};

    disp_handle = lvgl_port_add_disp(&disp_cfg);
    if (disp_handle == NULL) {
        ESP_LOGE(TAG, "Error al agregar la pantalla a LVGL Port");
        while (1) {}
    }

    xTaskCreate(view_colors_task, "colors_task", 4096, NULL, 5, NULL);

    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}