#include "lvgl.h"
#include "clock_view.h"
#include <atomic>
#include "controllers/button_manager/button_manager.h"
#include "controllers/screen_manager/screen_manager.h"
#include "esp_log.h"
#include <cstdlib>

static const char* TAG = "CLOCK_VIEW";

const int GRID_ROWS = 5;
const int GRID_COLS = 12;
const int CELL_SIZE = 10;
const int CELL_SPACING = 3;
const int CELL_RADIUS = 3;

static ClockView* currentClockView = nullptr; // Para update_time_task

ClockView::ClockView() : BaseView("Clock"), time_label(nullptr), grid(nullptr), grid_cells(), timer(nullptr),
                        hours(12), minutes(0), seconds(0)
{
    // Crear grid
    grid = lv_obj_create(screen);
    int grid_width = GRID_COLS * (CELL_SIZE + CELL_SPACING) - CELL_SPACING + 27;
    int grid_height = GRID_ROWS * (CELL_SIZE + CELL_SPACING) - CELL_SPACING + 27;
    lv_obj_set_size(grid, grid_width, grid_height);
    lv_obj_align(grid, LV_ALIGN_CENTER, 0, -40);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(grid, lv_color_hex(0xDDDDDD), LV_PART_MAIN);

    this->create_grid_cells();

    // Crear label de tiempo
    time_label = lv_label_create(screen);
    lv_label_set_text_fmt(time_label, "%02d:%02d:%02d", 12, 0, 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 80);

    timer = lv_timer_create(update_time_task, 1000, this);
    currentClockView = this; // Almacenar la instancia actual
}

ClockView::~ClockView() {
    // La limpieza se hace en destroy().
}

void ClockView::destroy() {
    if (timer) {
        lv_timer_del(timer);
        timer = nullptr;
    }
     if (currentClockView == this) { // Solo limpiar si es la instancia actual
        currentClockView = nullptr;
    }
    BaseView::destroy(); // Llamar a la clase base.
}

void ClockView::create_grid_cells() {
    grid_cells.resize(GRID_ROWS, std::vector<lv_obj_t*>(GRID_COLS));

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

void ClockView::update_grid_animation() {
    // Limpiar la cuadrícula si estamos al principio de un nuevo minuto (seconds == 0).
    if (seconds == 0) {
        for (auto& row : grid_cells) {
            for (auto& cell : row) {
                lv_obj_set_style_bg_color(cell, lv_color_white(), LV_PART_MAIN);
            }
        }
    }

    // Encender el cuadrado actual (basado en seconds).
     if (seconds > 0 && seconds < GRID_ROWS * GRID_COLS) { // Ya no se ilumina el cuadrado 0
        int row = (seconds % (GRID_ROWS * GRID_COLS)) / GRID_COLS;
        int col = (seconds % (GRID_ROWS * GRID_COLS)) % GRID_COLS;
        //Color aleatorio, pero se puede usar un color fijo:
        lv_color_t color = lv_color_make(rand() % 256, rand() % 256, rand() % 256);

        //Comprobación de seguridad
        if(row < GRID_ROWS && col < GRID_COLS)
            lv_obj_set_style_bg_color(grid_cells[row][col], color, LV_PART_MAIN);
    }
}

void ClockView::update_time_task(lv_timer_t*) {
    if (currentClockView) { // Verificar si currentClockView es válido.
        currentClockView->seconds++;
        if (currentClockView->seconds >= 60) {
            currentClockView->seconds = 0;
            currentClockView->minutes++;
            if (currentClockView->minutes >= 60) {
                currentClockView->minutes = 0;
                currentClockView->hours++;
                if (currentClockView->hours >= 24) {
                    currentClockView->hours = 0;
                }
            }
        }

        if (currentClockView->time_label) {
            lv_label_set_text_fmt(currentClockView->time_label, "%02d:%02d:%02d",
                                currentClockView->hours.load(),
                                currentClockView->minutes.load(),
                                currentClockView->seconds.load());
        }

        currentClockView->update_grid_animation();
    }
}
void ClockView::register_button_handlers() {
    button_manager_register_view_handler(BUTTON_1, []() {
        ESP_LOGI(TAG, "Botón 1 - Modo Reloj");
    });

    button_manager_register_view_handler(BUTTON_3, []() {
        ESP_LOGI(TAG, "Botón 3 - Cambiar Color");
    });

    button_manager_register_view_handler(BUTTON_4, []() {
        ESP_LOGI(TAG, "Botón 4 - Ir a Settings");
        switch_screen("Settings");
    });
}

void ClockView::unregister_button_handlers() {
    button_manager_unregister_view_handlers();
}