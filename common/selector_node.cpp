#include "selector_node.H"

namespace common {

EnumBehaviorTreeStatus SelectorNode::OnInvoke() {
    for (auto &child : child_nodes) {
        auto state = child->Invoke();
        if (state == EnumBehaviorTreeStatus::eSuccess)
            return EnumBehaviorTreeStatus::eSuccess;
        if (state == EnumBehaviorTreeStatus::eRunning)
            return EnumBehaviorTreeStatus::eRunning;
    }
    return EnumBehaviorTreeStatus::eFail;
}
} // namespace common
