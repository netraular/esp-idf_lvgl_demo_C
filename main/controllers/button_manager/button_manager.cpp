#include "controllers/button_manager/button_manager.h"
#include "esp_log.h"
#include <cassert>

static const char *TAG = "BTN_MGR";

static button_handle_t buttons[BUTTON_COUNT];
static button_handlers_t button_handlers[BUTTON_COUNT] = { {nullptr, nullptr} }; // Estructura para ambos handlers


static void default_button_left_handler() { ESP_LOGI(TAG, "Botón LEFT (Default)"); }
static void default_button_cancel_handler() { ESP_LOGI(TAG, "Botón CANCEL (Default)"); }
static void default_button_ok_handler() { ESP_LOGI(TAG, "Botón OK (Default)"); }
static void default_button_right_handler() { ESP_LOGI(TAG, "Botón RIGHT (Default)"); }
static void default_button_on_off_handler() { ESP_LOGI(TAG, "Botón ON/OFF (Default)"); }

void button_manager_init() {
    button_config_t btn_config = {
        .long_press_time = 0,
        .short_press_time = 0,
    };
    
    const button_gpio_config_t gpio_config[BUTTON_COUNT] = {
        {BUTTON_LEFT_PIN, 0, false, false},   // BUTTON_LEFT
        {BUTTON_CANCEL_PIN, 0, false, false}, // BUTTON_CANCEL
        {BUTTON_OK_PIN, 0, false, false},     // BUTTON_OK
        {BUTTON_RIGHT_PIN, 0, false, false},  // BUTTON_RIGHT
        {BUTTON_ON_OFF_PIN, 0, false, false}  // BUTTON_ON_OFF
    };

    for (int i = 0; i < BUTTON_COUNT; i++) {
        ESP_ERROR_CHECK(iot_button_new_gpio_device(&btn_config, &gpio_config[i], &buttons[i]));
    }
    
    // Registrar handlers por defecto
    button_manager_register_default_handler(BUTTON_LEFT, default_button_left_handler);
    button_manager_register_default_handler(BUTTON_CANCEL, default_button_cancel_handler);
    button_manager_register_default_handler(BUTTON_OK, default_button_ok_handler);
    button_manager_register_default_handler(BUTTON_RIGHT, default_button_right_handler);
    button_manager_register_default_handler(BUTTON_ON_OFF, default_button_on_off_handler);
}

void button_manager_register_default_handler(button_id_t button, button_handler_t handler) {
    if (button < BUTTON_COUNT) {
        ESP_LOGI(TAG, "Registrando handler por defecto para el botón %d", button);
        button_handlers[button].default_handler = handler;
        
        // Registrar solo si no hay handler de vista
        if (!button_handlers[button].view_handler) {
            esp_err_t err = iot_button_register_cb(
                buttons[button], 
                BUTTON_SINGLE_CLICK,
                NULL,
                [](void* arg, void* usr_data) {
                    ((button_handler_t)usr_data)();
                },
                (void*)handler
            );
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Error al registrar handler por defecto: %s", esp_err_to_name(err));
            } else {
                ESP_LOGI(TAG, "Handler por defecto registrado correctamente para el botón %d", button);
            }
        } else {
            ESP_LOGI(TAG, "No se registra handler por defecto para el botón %d porque ya hay un handler de vista", button);
        }
    }
}

void button_manager_register_view_handler(button_id_t button, button_handler_t handler) {
    if (button < BUTTON_COUNT) {
        ESP_LOGI(TAG, "Registrando handler de vista para el botón %d", button);

        // 1. Desregistrar handler por defecto si está registrado
        if (button_handlers[button].default_handler) {
            esp_err_t err = iot_button_unregister_cb(buttons[button], BUTTON_SINGLE_CLICK, NULL);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Error al desregistrar handler por defecto: %s", esp_err_to_name(err));
            } else {
                ESP_LOGI(TAG, "Handler por defecto desregistrado correctamente para el botón %d", button);
            }
        }

        // 2. Registrar nuevo handler de vista
        button_handlers[button].view_handler = handler;
        esp_err_t err = iot_button_register_cb(
            buttons[button], 
            BUTTON_SINGLE_CLICK,
            NULL,
            [](void* arg, void* usr_data) {
                ((button_handler_t)usr_data)();
            },
            (void*)handler
        );
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error al registrar handler de vista: %s", esp_err_to_name(err));
        } else {
            ESP_LOGI(TAG, "Handler de vista registrado correctamente para el botón %d", button);
        }
    }
}

void button_manager_unregister_view_handlers() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (button_handlers[i].view_handler) {
            ESP_LOGI(TAG, "Desregistrando handler de vista para el botón %d", i);

            // 1. Eliminar handler de vista
            esp_err_t err = iot_button_unregister_cb(buttons[i], BUTTON_SINGLE_CLICK, NULL);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Error al desregistrar handler de vista: %s", esp_err_to_name(err));
            } else {
                ESP_LOGI(TAG, "Handler de vista desregistrado correctamente para el botón %d", i);
            }
            button_handlers[i].view_handler = nullptr;
            
            // 2. Restaurar handler por defecto si existe
            if (button_handlers[i].default_handler) {
                err = iot_button_register_cb(
                    buttons[i], 
                    BUTTON_SINGLE_CLICK,
                    NULL,
                    [](void* arg, void* usr_data) {
                        ((button_handler_t)usr_data)();
                    },
                    (void*)button_handlers[i].default_handler
                );
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Error al restaurar handler por defecto: %s", esp_err_to_name(err));
                } else {
                    ESP_LOGI(TAG, "Handler por defecto restaurado correctamente para el botón %d", i);
                }
            }
        }
    }
}