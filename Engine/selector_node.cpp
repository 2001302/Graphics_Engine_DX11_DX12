#include "selector_node.H"

namespace engine {

EnumBehaviorTreeStatus SelectorNode::OnInvoke() {
    for (auto &child : child_nodes) {
        if (child->Invoke() == EnumBehaviorTreeStatus::eSuccess)
            return EnumBehaviorTreeStatus::eSuccess;
    }
    return EnumBehaviorTreeStatus::eFail;
}
} // namespace engine