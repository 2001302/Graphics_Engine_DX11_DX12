#ifndef _BLACKBOARD
#define _BLACKBOARD

#include "render_info.h"

namespace core {

struct BlackBoard : public foundation::IDataBlock {
    BlackBoard() {
        targets = std::make_shared<RenderTargetObject>();
        conditions = std::make_shared<RenderCondition>();
    }
    std::shared_ptr<RenderTargetObject> targets;
    std::shared_ptr<RenderCondition> conditions;
};

} // namespace core
#endif
