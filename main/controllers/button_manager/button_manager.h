#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include "iot_button.h"
#include "button_gpio.h"
#include "config.h"

typedef enum {
    BUTTON_1 = 0,  // Primer botón
    BUTTON_2,      // Valor 1
    BUTTON_3,      // Valor 2
    BUTTON_4,      // Valor 3
    BUTTON_COUNT   // Valor 4 (total de botones)
} button_id_t;

typedef void (*button_handler_t)(void);

// Nuevo: Tipo para almacenar ambos tipos de handlers
typedef struct {
    button_handler_t default_handler;
    button_handler_t view_handler;
} button_handlers_t;

void button_manager_init();
void button_manager_register_default_handler(button_id_t button, button_handler_t handler);
void button_manager_register_view_handler(button_id_t button, button_handler_t handler);
void button_manager_unregister_view_handlers();

#endif