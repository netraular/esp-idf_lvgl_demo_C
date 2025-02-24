#ifndef BASE_VIEW_H
#define BASE_VIEW_H

#include "lvgl.h"

typedef struct {
    lv_obj_t* screen;
    const char* name; // Nombre de la vista para identificarla
    void (*register_button_handlers)(void);  // Puntero a función para registrar handlers
    void (*unregister_button_handlers)(void); // Puntero a función para desregistrar handlers
    void (*destroy)(void); // Nueva función para destruir la vista
} base_view_t;

lv_obj_t* create_base_view(const char* name);
void destroy_base_view(lv_obj_t* screen);

#endif
