#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include "iot_button.h"
#include "button_gpio.h"
#include "config.h"

typedef enum {
    BUTTON_LEFT = 0,
    BUTTON_CANCEL,    
    BUTTON_OK,
    BUTTON_RIGHT,
    BUTTON_ON_OFF,
    BUTTON_COUNT
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