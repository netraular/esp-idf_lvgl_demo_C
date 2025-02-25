#ifndef SETTINGS_VIEW_H
#define SETTINGS_VIEW_H

#include "../../base_view.h"

class SettingsView : public BaseView {
private:
    lv_obj_t* label;

public:
    SettingsView();
    virtual ~SettingsView();

    void register_button_handlers() override;
    void unregister_button_handlers() override;
};

#endif