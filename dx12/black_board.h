#ifndef _BLACKBOARD
#define _BLACKBOARD

#include "render_condition.h"
#include "render_target_object.h"
#include <input.h>
#include <setting_ui.h>

namespace graphics {

struct BlackBoard : public common::IDataBlock {
    BlackBoard() {
        targets = std::make_shared<RenderTargetObject>();
        conditions = std::make_shared<RenderCondition>();
        input = std::make_unique<common::Input>();
        gui = std::make_shared<common::SettingUi>();
    }
    std::shared_ptr<RenderTargetObject> targets;
    std::shared_ptr<RenderCondition> conditions;
    std::shared_ptr<common::Input> input;
    std::shared_ptr<common::SettingUi> gui;
    // gpu
};

} // namespace graphics
#endif
