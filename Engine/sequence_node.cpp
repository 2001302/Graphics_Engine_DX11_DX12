#include "sequence_node.H"

using namespace dx11;

EnumBehaviorTreeStatus SequenceNode::OnInvoke() {
    for (auto &child : child_nodes) {
        if (child->Invoke() == EnumBehaviorTreeStatus::eFail)
            return EnumBehaviorTreeStatus::eFail;
    }
    return EnumBehaviorTreeStatus::eSuccess;
}
