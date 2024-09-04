#ifndef _BLACKBOARD
#define _BLACKBOARD

#include "render_condition.h"
#include "render_target_object.h"
#include "foundation/setting_ui.h"
#include "foundation/input.h"

namespace core {

struct BlackBoard : public foundation::IDataBlock {
    BlackBoard() {
        targets = std::make_shared<RenderTargetObject>();
        conditions = std::make_shared<RenderCondition>();
        input = std::make_unique<foundation::Input>();
        gui = std::make_shared<foundation::SettingUi>();
    }
    std::shared_ptr<RenderTargetObject> targets;
    std::shared_ptr<RenderCondition> conditions;
    std::shared_ptr<foundation::Input> input;
    std::shared_ptr<foundation::SettingUi> gui;
    //gpu
};

} // namespace core
#endif
