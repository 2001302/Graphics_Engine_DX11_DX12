#include "action_node.h"

namespace foundation {

EnumBehaviorTreeStatus BehaviorActionNode::Invoke() {
    if (parent_node)
        this->target_object = parent_node->target_object;
    return OnInvoke();
}

EnumBehaviorTreeStatus BehaviorActionNode::Reset() {
    for (auto &child : child_nodes) {
        child->Reset();
    }
    return OnReset();
}

EnumBehaviorTreeStatus BehaviorActionNode::OnInvoke() {
    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus BehaviorActionNode::OnReset() {
    return EnumBehaviorTreeStatus::eSuccess;
}

void BehaviorActionNode::PushNode(std::shared_ptr<BehaviorActionNode> node) {
    node->parent_node = this;
    node->data_block = data_block;
    child_nodes.push_back(node);
}

BehaviorActionNode *BehaviorActionNode::GetParent() { return parent_node; }

void BehaviorActionNode::PopNode() { child_nodes.pop_back(); }

} // namespace common
