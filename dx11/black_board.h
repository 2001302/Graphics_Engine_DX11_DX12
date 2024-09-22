#ifndef _BLACKBOARD
#define _BLACKBOARD

#include "job_context.h"
#include <input.h>
#include <setting_ui.h>

namespace graphics {

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

} // namespace graphics
#endif
