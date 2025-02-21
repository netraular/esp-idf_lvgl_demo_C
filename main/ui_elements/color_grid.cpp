#include "color_grid.h"
#include "lvgl.h"
// Definimos los colores como macros
constexpr uint32_t MY_COLOR_RED = 0xFF0000;
constexpr uint32_t MY_COLOR_GREEN = 0x00FF00;
constexpr uint32_t MY_COLOR_BLUE = 0x0000FF;
constexpr uint32_t MY_COLOR_YELLOW = 0xFFFF00;

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

lv_obj_t* create_color_grid(lv_obj_t *parent) {
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
    for (std::size_t i = 0; i < 4; i++) {
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
    return cont;
}