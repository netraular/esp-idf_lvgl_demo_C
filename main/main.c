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
//static lv_obj_t *scr = NULL; //  No necesario ahora, usamos lv_scr_act()

// Definimos los colores como macros
#define MY_COLOR_RED    0xFF0000
#define MY_COLOR_GREEN  0x00FF00
#define MY_COLOR_BLUE   0x0000FF
#define MY_COLOR_YELLOW 0xFFFF00

// Estructura para asociar color y nombre (global para usarla en app_main)
struct ColorInfo {
    uint32_t color;
    const char *name;
};

// Array de colores (global para usarlo en app_main)
static const struct ColorInfo palette_colors[] = {
    {MY_COLOR_RED,   "Red"},
    {MY_COLOR_GREEN, "Green"},
    {MY_COLOR_BLUE,  "Blue"},
    {MY_COLOR_YELLOW,"Yellow"},
};


// Función para crear las casillas (ahora toma la pantalla como argumento)
static void create_color_grid(lv_obj_t *parent) {
    // Crear un contenedor para organizar las casillas
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 240, 240);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID); // Usamos un grid layout

    // Definir las columnas (2 columnas de igual tamaño)
    static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    // Definir las filas (2 filas de igual tamaño)
    static lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);

    // Crear las 4 casillas
    for (size_t i = 0; i < 4; i++) {
        lv_obj_t *cell = lv_obj_create(cont);
        lv_obj_set_size(cell, 120, 120); // Tamaño de cada casilla

        // Calcular fila y columna
        int row = i / 2;
        int col = i % 2;
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, col, 1,
                             LV_GRID_ALIGN_STRETCH, row, 1);


        lv_obj_set_style_bg_color(cell, lv_color_hex(palette_colors[i].color), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, LV_PART_MAIN);

        // Añadir el texto
        lv_obj_t *label = lv_label_create(cell);
        lv_label_set_text(label, palette_colors[i].name);
        lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN); // Texto en negro
        lv_obj_center(label); // Centrar el texto dentro de la casilla
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
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .flags = {}
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = TFT_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, // Corregido a RGB
        .bits_per_pixel = 16,
        .flags = {},
        .vendor_config = NULL
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));

    if (TFT_BL >= 0) {
        gpio_config_t bk_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << TFT_BL,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
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
    .flags = {
			.buff_dma = 1,
			.swap_bytes = 1, //Añadido swap_bytes
		}
	};

    disp_handle = lvgl_port_add_disp(&disp_cfg);
    if (disp_handle == NULL) {
        ESP_LOGE(TAG, "Error al agregar la pantalla a LVGL Port");
        while (1) {}
    }

    // Obtenemos la pantalla activa (no es necesario hacerla global)
    lv_obj_t *scr = lv_disp_get_scr_act(disp_handle);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN); // Fondo opaco

    // Crear la cuadrícula de colores
    create_color_grid(scr);


    // --- Bucle principal (solo llama a lv_timer_handler) ---
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}