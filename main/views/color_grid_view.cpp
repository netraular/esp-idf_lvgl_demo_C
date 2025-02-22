// color_grid_view.cpp
#include "color_grid_view.h"

// Definimos los colores como macros (podrían estar en un archivo de configuración común)
constexpr uint32_t MY_COLOR_RED = 0xFF0000;
constexpr uint32_t MY_COLOR_GREEN = 0x00FF00;
constexpr uint32_t MY_COLOR_BLUE = 0x0000FF;
constexpr uint32_t MY_COLOR_YELLOW = 0xFFFF00;

struct ColorInfo {
    uint32_t color;
    const char* name;
};

static const struct ColorInfo palette_colors[] = {
    {MY_COLOR_RED, "Red"},
    {MY_COLOR_GREEN, "Green"},
    {MY_COLOR_BLUE, "Blue"},
    {MY_COLOR_YELLOW, "Yellow"},
};

static lv_obj_t *number_label; // Label to display the number
static int current_number = 0;    // Variable to store the number

// Timer callback to increment the number
static void increment_number_task(lv_timer_t *timer) {
    current_number++;
    lv_label_set_text_fmt(number_label, "%d", current_number);
}

lv_obj_t* create_color_grid_view(lv_obj_t* parent) {
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 240, 240);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    static lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);

    for (size_t i = 0; i < 4; i++) {
        lv_obj_t* cell = lv_obj_create(cont);
        lv_obj_set_size(cell, 120, 120);
        int row = i / 2;
        int col = i % 2;
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
        lv_obj_set_style_bg_color(cell, lv_color_hex(palette_colors[i].color), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, LV_PART_MAIN);

        lv_obj_t* label = lv_label_create(cell);
        lv_label_set_text(label, palette_colors[i].name);
        lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
        lv_obj_center(label);
    }

    // Create the label to display the number
    number_label = lv_label_create(cont);
    lv_label_set_text_fmt(number_label, "%d", current_number); // Initial value
    lv_obj_set_style_text_font(number_label, &lv_font_montserrat_48, LV_PART_MAIN); // Use a larger font
    lv_obj_set_style_text_color(number_label, lv_color_black(), LV_PART_MAIN); // Ensure text color is visible

    //Position the number label in the center.  Crucial for correct display!
    lv_obj_set_grid_cell(number_label, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 2);


    // Create a timer to increment the number every second
    lv_timer_create(increment_number_task, 1000, nullptr);

    // Reset current_number to 0 whenever the view is created.  Important!
    current_number = 0;

    return cont;
}