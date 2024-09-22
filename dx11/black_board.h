#ifndef _BLACKBOARD
#define _BLACKBOARD

#include "input.h"
#include "job_context.h"
#include "setting_ui.h"

namespace graphics {

struct BlackBoard : public foundation::IDataBlock {
    BlackBoard() {
        job_context = std::make_shared<JobContext>();
        input = std::make_unique<foundation::Input>();
        gui = std::make_shared<foundation::SettingUi>();
    }
    std::shared_ptr<JobContext> job_context;
    std::shared_ptr<foundation::Input> input;
    std::shared_ptr<foundation::SettingUi> gui;
};

} // namespace core
#endif
