#include "sequence_node.H"

namespace engine {

EnumBehaviorTreeStatus SequenceNode::OnInvoke() {
    for (auto &child : child_nodes) {
        if (child->Invoke() == EnumBehaviorTreeStatus::eFail)
            return EnumBehaviorTreeStatus::eFail;
    }
    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace engine