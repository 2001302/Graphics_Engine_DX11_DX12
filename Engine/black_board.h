#ifndef _BLACKBOARD
#define _BLACKBOARD

#include "setting_ui.h"
#include "input.h"
#include "rendering_block.h"

namespace engine {

struct BlackBoard : public common::IDataBlock {
    BlackBoard() {
        render_block = std::make_shared<RenderingBlock>();
        input = std::make_unique<Input>();
        gui = std::make_shared<common::SettingUi>();
    }
    std::shared_ptr<RenderingBlock> render_block;
    std::shared_ptr<Input> input;
    std::shared_ptr<common::SettingUi> gui;
};

} // namespace engine
#endif
