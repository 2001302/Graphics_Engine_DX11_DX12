#include "sequence_node.H"

namespace foundation {

EnumBehaviorTreeStatus SequenceNode::OnInvoke() {
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