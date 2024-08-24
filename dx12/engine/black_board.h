#ifndef _BLACKBOARD
#define _BLACKBOARD

#include "render_info.h"

namespace core {

struct BlackBoard : public foundation::IDataBlock {
    BlackBoard() {
        render_targets = std::make_shared<RenderTarget>();
        render_condition = std::make_shared<RenderCondition>();
    }
    std::shared_ptr<RenderTarget> render_targets;
    std::shared_ptr<RenderCondition> render_condition;
};

} // namespace core
#endif
