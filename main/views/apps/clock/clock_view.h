// main/views/apps/clock/clock_view.h
#ifndef CLOCK_VIEW_H
#define CLOCK_VIEW_H

#include "../../base_view.h"
#include <atomic>
#include <vector>
#include "lvgl.h"

class ClockView : public BaseView {
private:
    lv_obj_t* time_label;
    lv_obj_t* grid;
    std::vector<std::vector<lv_obj_t*>> grid_cells;
    lv_timer_t* timer;
    std::atomic<int> hours;
    std::atomic<int> minutes;
    std::atomic<int> seconds;

    void create_grid_cells();
    void update_grid_animation();
    static void update_time_task(lv_timer_t* t);

    // Para el patrón Singleton
    static ClockView* instance;
    ClockView(); // Constructor privado

public:
    // Método estático para obtener la instancia (Singleton)
    static ClockView* get_instance();
    virtual ~ClockView();
    void register_button_handlers() override;
    void unregister_button_handlers() override;
    void destroy() override;
};

#endif