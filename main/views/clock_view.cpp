#include "clock_view.h"
#include "esp_log.h"
#include <atomic>
#include <cmath>

static const char *TAG = "CLOCK";

// Variables de tiempo
static std::atomic<int> hours(12);
static std::atomic<int> minutes(0);
static std::atomic<int> seconds(0);
static lv_timer_t *timer = nullptr;

// Objetos LVGL
static lv_obj_t *time_label = nullptr;
static lv_obj_t *grid = nullptr;
static lv_obj_t *grid_cells[5][12] = {nullptr};  // 5 filas y 12 columnas

// Tamaño de la cuadrícula
const int GRID_ROWS = 5;  // 5 filas
const int GRID_COLS = 12; // 12 columnas
const int CELL_SIZE = 10; // Tamaño de cada celda en px
const int CELL_SPACING = 3; // Separación entre las celdas en px
const int CELL_RADIUS = 3;   // Radio para las esquinas redondeadas

// Función para obtener un color aleatorio
lv_color_t get_random_color() {
    uint8_t r = rand() % 256;
    uint8_t g = rand() % 256;
    uint8_t b = rand() % 256;
    return lv_color_make(r, g, b);
}

// Crear la cuadrícula con celdas inicializadas en blanco
void create_grid_cells() {
    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            // Crear un objeto de tipo cuadrado (celda) para cada celda
            lv_obj_t* cell = lv_obj_create(grid);
            lv_obj_set_size(cell, CELL_SIZE, CELL_SIZE);  // Tamaño de cada celda

            // Posicionar las celdas con separación entre ellas
            lv_obj_align(cell, LV_ALIGN_TOP_LEFT, 
                         j * (CELL_SIZE + CELL_SPACING), 
                         i * (CELL_SIZE + CELL_SPACING));
            
            lv_obj_set_style_bg_color(cell, lv_color_white(), LV_PART_MAIN); // Color blanco inicial
            lv_obj_set_style_border_width(cell, 1, LV_PART_MAIN); // Borde de 1 px
            lv_obj_set_style_border_color(cell, lv_color_black(), LV_PART_MAIN); // Borde negro
            lv_obj_set_style_radius(cell, CELL_RADIUS, LV_PART_MAIN); // Esquinas redondeadas

            grid_cells[i][j] = cell;  // Guardamos la celda
        }
    }
}

// Actualizar la cuadrícula con un nuevo cuadrado cada segundo
void update_grid_animation() {
    static int cell_count = 0;

    if (cell_count >= GRID_ROWS * GRID_COLS) {
        // Reiniciar la cuadrícula cuando llegue a 60 cuadros
        for (int i = 0; i < GRID_ROWS; i++) {
            for (int j = 0; j < GRID_COLS; j++) {
                lv_obj_set_style_bg_color(grid_cells[i][j], lv_color_white(), LV_PART_MAIN);  // Borrar el color (blanco)
            }
        }
        cell_count = 0;
    }

    // Calcular fila y columna actual
    int row = cell_count / GRID_COLS;
    int col = cell_count % GRID_COLS;

    // Asignar color aleatorio a la celda
    lv_obj_set_style_bg_color(grid_cells[row][col], get_random_color(), LV_PART_MAIN);
    
    cell_count++;
}

// Callback para actualizar el tiempo y la animación
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

    // Actualizar animación de la cuadrícula
    update_grid_animation();
}

// Crear la cuadrícula de animación
static void create_grid(lv_obj_t* parent) {
    grid = lv_obj_create(parent);
    
    // Ajustar tamaño del contenedor de la cuadrícula (con las celdas y separaciones)
    int grid_width = GRID_COLS * (CELL_SIZE + CELL_SPACING) - CELL_SPACING + 27;  // Ancho total + padding
    int grid_height = GRID_ROWS * (CELL_SIZE + CELL_SPACING) - CELL_SPACING + 27;  // Alto total + padding
    lv_obj_set_size(grid, grid_width, grid_height);
    
    // Alinear el grid en el centro y con un pequeño margen hacia arriba para dejar espacio al reloj
    lv_obj_align(grid, LV_ALIGN_CENTER, 0, -40);

    // Desactivar el desplazamiento (scrollbars)
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(grid, lv_color_hex(0xDDDDDD), LV_PART_MAIN);  // Color de fondo

    // Crear las celdas de la cuadrícula
    create_grid_cells();
}

// Handlers de botón específicos de esta vista
    static void clock_view_button1_handler(void* arg, void* usr_data) {
        ESP_LOGI(TAG, "Botón 1 presionado en vista de reloj");
    }

    static void clock_view_button2_handler(void* arg, void* usr_data) {
        ESP_LOGI(TAG, "Botón 2 presionado en vista de reloj");
    }

    static void clock_view_button3_handler(void* arg, void* usr_data) {
        ESP_LOGI(TAG, "Botón 3 presionado en vista de reloj");
    }

    static void clock_view_button4_handler(void* arg, void* usr_data) {
        ESP_LOGI(TAG, "Botón 4 presionado en vista de reloj");
    }

// Registrar callbacks de botón cuando se crea la vista
    void clock_view_register_button_handlers() {
        iot_button_register_cb(button1, BUTTON_SINGLE_CLICK, NULL, clock_view_button1_handler, NULL);
        ESP_LOGI(TAG, "Callback del botón 1 registrado en vista de reloj");
        iot_button_register_cb(button2, BUTTON_SINGLE_CLICK, NULL, clock_view_button2_handler, NULL);
        ESP_LOGI(TAG, "Callback del botón 2 registrado en vista de reloj");
        iot_button_register_cb(button3, BUTTON_SINGLE_CLICK, NULL, clock_view_button3_handler, NULL);
        ESP_LOGI(TAG, "Callback del botón 3 registrado en vista de reloj");
        iot_button_register_cb(button4, BUTTON_SINGLE_CLICK, NULL, clock_view_button4_handler, NULL);
        ESP_LOGI(TAG, "Callback del botón 4 registrado en vista de reloj");
    }

// Eliminar callbacks de botón cuando se destruye la vista
    void clock_view_unregister_button_handlers() {
            iot_button_unregister_cb(button1, BUTTON_SINGLE_CLICK, NULL);
            ESP_LOGI(TAG, "Callback del botón 1 desregistrado en vista de reloj");
            iot_button_unregister_cb(button2, BUTTON_SINGLE_CLICK, NULL);
            ESP_LOGI(TAG, "Callback del botón 2 desregistrado en vista de reloj");
            iot_button_unregister_cb(button3, BUTTON_SINGLE_CLICK, NULL);
            ESP_LOGI(TAG, "Callback del botón 3 desregistrado en vista de reloj");
            iot_button_unregister_cb(button4, BUTTON_SINGLE_CLICK, NULL);
            ESP_LOGI(TAG, "Callback del botón 4 desregistrado en vista de reloj");
    }

// Crear la vista principal
lv_obj_t* create_clock_view(lv_obj_t* parent) {
    ESP_LOGI(TAG, "Creando vista del reloj");

    // Limpiar timers anteriores
    if (timer) {
        lv_timer_del(timer);
        timer = nullptr;
    }

    // Crear contenedor principal
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 240, 240);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    // Crear cuadrícula animada
    create_grid(cont);

    // Crear display digital
    time_label = lv_label_create(cont);
    lv_label_set_text_fmt(time_label, "%02d:%02d:%02d", 12, 0, 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 80);

    // Inicializar tiempo
    static bool first_init = true;
    if (first_init) {
        hours = 12;
        minutes = 0;
        seconds = 0;
        first_init = false;
    }

    // Iniciar timer
    timer = lv_timer_create(update_time_task, 1000, nullptr);

    // Registrar eventos de botón
    clock_view_register_button_handlers();
    
    // Configurar limpieza al destruir la vista
    lv_obj_add_event_cb(cont, [](lv_event_t* e) {
        clock_view_unregister_button_handlers();
        ESP_LOGI(TAG, "Vista de reloj destruida - botones desregistrados");
    }, LV_EVENT_DELETE, NULL);

    return cont;
}