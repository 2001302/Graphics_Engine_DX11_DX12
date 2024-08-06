#ifndef _BLACKBOARD
#define _BLACKBOARD

#include "setting_ui.h"
#include "input.h"
#include "job_context.h"

namespace engine {

struct BlackBoard : public common::IDataBlock {
    BlackBoard() {
        render_block = std::make_shared<JobContext>();
        input = std::make_unique<common::Input>();
        gui = std::make_shared<SettingUi>();
    }
    std::shared_ptr<JobContext> render_block;
    std::shared_ptr<common::Input> input;
    std::shared_ptr<SettingUi> gui;
};

} // namespace engine
#endif
