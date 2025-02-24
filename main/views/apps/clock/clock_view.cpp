#include "lvgl.h"
#include "views/base_view.h"
#include "clock_view.h"
#include <atomic>
#include <cmath>
#include "controllers/button_manager/button_manager.h"
#include "controllers/screen_manager/screen_manager.h"
#include "esp_log.h"

static const char *TAG = "CLOCK";

// Variables de tiempo
static std::atomic<int> hours(12);
static std::atomic<int> minutes(0);
static std::atomic<int> seconds(0);
static lv_timer_t *timer = nullptr;

// Objetos LVGL
static lv_obj_t *time_label = nullptr;
static lv_obj_t *grid = nullptr;
static lv_obj_t *grid_cells[5][12] = {nullptr};

// Tamaño de la cuadrícula
const int GRID_ROWS = 5;
const int GRID_COLS = 12;
const int CELL_SIZE = 10;
const int CELL_SPACING = 3;
const int CELL_RADIUS = 3;

lv_color_t get_random_color() {
    uint8_t r = rand() % 256;
    uint8_t g = rand() % 256;
    uint8_t b = rand() % 256;
    return lv_color_make(r, g, b);
}

void create_grid_cells() {
    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            lv_obj_t* cell = lv_obj_create(grid);
            lv_obj_set_size(cell, CELL_SIZE, CELL_SIZE);
            lv_obj_align(cell, LV_ALIGN_TOP_LEFT,
                         j * (CELL_SIZE + CELL_SPACING),
                         i * (CELL_SIZE + CELL_SPACING));
            lv_obj_set_style_bg_color(cell, lv_color_white(), LV_PART_MAIN);
            lv_obj_set_style_border_width(cell, 1, LV_PART_MAIN);
            lv_obj_set_style_border_color(cell, lv_color_black(), LV_PART_MAIN);
            lv_obj_set_style_radius(cell, CELL_RADIUS, LV_PART_MAIN);
            grid_cells[i][j] = cell;
        }
    }
}

void update_grid_animation() {
    static int cell_count = 0;

    if (cell_count >= GRID_ROWS * GRID_COLS) {
        for (int i = 0; i < GRID_ROWS; i++) {
            for (int j = 0; j < GRID_COLS; j++) {
                lv_obj_set_style_bg_color(grid_cells[i][j], lv_color_white(), LV_PART_MAIN);
            }
        }
        cell_count = 0;
    }

    int row = cell_count / GRID_COLS;
    int col = cell_count % GRID_COLS;
    lv_obj_set_style_bg_color(grid_cells[row][col], get_random_color(), LV_PART_MAIN);
    cell_count++;
}

static void update_time_task(lv_timer_t *t) {
    seconds++;
    if (seconds >= 60) {
        seconds = 0;
        minutes++;
        if (minutes >= 60) {
            minutes = 0;
            hours++;
            if (hours >= 24) {
                hours = 0;
            }
        }
    }
    if (time_label) {
        lv_label_set_text_fmt(time_label, "%02d:%02d:%02d",
                            hours.load(), minutes.load(), seconds.load());
    }
    update_grid_animation();
}

static void create_grid(lv_obj_t* parent) {
    grid = lv_obj_create(parent);
    int grid_width = GRID_COLS * (CELL_SIZE + CELL_SPACING) - CELL_SPACING + 27;
    int grid_height = GRID_ROWS * (CELL_SIZE + CELL_SPACING) - CELL_SPACING + 27;
    lv_obj_set_size(grid, grid_width, grid_height);
    lv_obj_align(grid, LV_ALIGN_CENTER, 0, -40);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(grid, lv_color_hex(0xDDDDDD), LV_PART_MAIN);
    create_grid_cells();
}

static void clock_button1_handler() { ESP_LOGI(TAG, "Botón 1 - Modo Reloj"); }
static void clock_button3_handler() { ESP_LOGI(TAG, "Botón 3 - Cambiar Color"); }
static void clock_button4_handler() {
    ESP_LOGI(TAG, "Botón 4 - Ir a Settings");
    destroy_clock_view(); // Limpiar vista antes de cambiar
    switch_screen("Settings");
}

static void clock_view_register_button_handlers() {
    button_manager_register_view_handler(BUTTON_1, clock_button1_handler);
    button_manager_register_view_handler(BUTTON_3, clock_button3_handler);
    button_manager_register_view_handler(BUTTON_4, clock_button4_handler);
}

static void clock_view_unregister_button_handlers() {
     button_manager_unregister_view_handlers();
}

static base_view_t clock_view;

static void init_base_view(base_view_t* view, const char* name, void (*register_handlers)(void), void (*unregister_handlers)(void), void (*destroy)(void)) {
    if (view) {
    view->name = name;
    view->register_button_handlers = register_handlers;
    view->unregister_button_handlers = unregister_handlers;
    view->destroy = destroy; // Se asigna la función de destrucción
    view->screen = create_base_view(name);
    lv_obj_set_user_data(view->screen, view);
    }
}



lv_obj_t* create_clock_view() {
    ESP_LOGI(TAG, "Creando vista del reloj");

    init_base_view(&clock_view, "Clock",
                   clock_view_register_button_handlers,
                   clock_view_unregister_button_handlers,
                   destroy_clock_view); // Se asigna la función destroy

    create_grid(clock_view.screen);
    time_label = lv_label_create(clock_view.screen);
    lv_label_set_text_fmt(time_label, "%02d:%02d:%02d", 12, 0, 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 80);

    static bool first_init = true;
    if (first_init) {
        hours = 12;
        minutes = 0;
        seconds = 0;
        first_init = false;
    }

    timer = lv_timer_create(update_time_task, 1000, nullptr);

    return clock_view.screen;
}


void destroy_clock_view() {
    ESP_LOGI(TAG, "Destruyendo vista del reloj");

    if (timer) {
        lv_timer_del(timer);
        timer = nullptr;
    }

    if (clock_view.screen) {
        lv_obj_clean(clock_view.screen);
        clock_view.screen = nullptr;
    }
}

