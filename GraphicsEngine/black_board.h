#ifndef _BLACKBOARD
#define _BLACKBOARD

#include "setting_ui.h"
#include "input.h"
#include "job_context.h"

namespace engine {

struct BlackBoard : public common::IDataBlock {
    BlackBoard() {
        job_context = std::make_shared<JobContext>();
        input = std::make_unique<common::Input>();
        gui = std::make_shared<common::SettingUi>();
    }
    std::shared_ptr<JobContext> job_context;
    std::shared_ptr<common::Input> input;
    std::shared_ptr<common::SettingUi> gui;
};

} // namespace engine
#endif
