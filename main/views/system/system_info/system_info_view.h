#ifndef SYSTEM_INFO_VIEW_H
#define SYSTEM_INFO_VIEW_H

#include "../../base_view.h"

class SystemInfoView : public BaseView {
private:
    lv_obj_t* label;

public:
    SystemInfoView();
    virtual ~SystemInfoView();

    void register_button_handlers() override;
    void unregister_button_handlers() override;
};

#endif