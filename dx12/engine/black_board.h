#ifndef _BLACKBOARD
#define _BLACKBOARD

#include "../foundation/setting_ui.h"
#include "../graphics/command_pool.h"
#include "input.h"
#include "job_context.h"

namespace core {

struct BlackBoard : public foundation::IDataBlock {
    BlackBoard() {
        job_context = std::make_shared<JobContext>();
        input = std::make_unique<foundation::Input>();
        gui = std::make_shared<foundation::SettingUi>();
        command_pool = std::make_shared<dx12::CommandPool>();
    }
    std::shared_ptr<JobContext> job_context;
    std::shared_ptr<foundation::Input> input;
    std::shared_ptr<foundation::SettingUi> gui;
    std::shared_ptr<dx12::CommandPool> command_pool;
};

} // namespace core
#endif
