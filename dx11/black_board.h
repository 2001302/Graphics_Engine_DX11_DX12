#ifndef _BLACKBOARD
#define _BLACKBOARD

#include "render_target_object.h"
#include <input.h>
#include <setting_ui.h>

namespace graphics {

struct BlackBoard : public common::IDataBlock {
    BlackBoard() {
        targets = std::make_shared<RenderTargetObject>();
        input = std::make_unique<common::Input>();
        gui = std::make_shared<common::SettingUi>();
    }
    std::shared_ptr<RenderTargetObject> targets;
    std::shared_ptr<common::Input> input;
    std::shared_ptr<common::SettingUi> gui;
};

} // namespace graphics
#endif
