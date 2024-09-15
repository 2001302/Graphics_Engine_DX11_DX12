#include "conditional_node.h"

namespace common {

EnumBehaviorTreeStatus ConditionalNode::OnInvoke() {
    for (auto &child : child_nodes) {
        auto state = child->Invoke();
        if (state == EnumBehaviorTreeStatus::eFail)
            return EnumBehaviorTreeStatus::eFail;
        if (state == EnumBehaviorTreeStatus::eRunning)
            return EnumBehaviorTreeStatus::eRunning;
    }
    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace common
