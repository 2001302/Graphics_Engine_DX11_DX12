#include "behavior_tree.h"

using namespace Engine;

EnumBehaviorTreeStatus BehaviorActionNode::Invoke() {
    if (parent_node)
        this->target_id = parent_node->target_id;
    return OnInvoke();
}

BehaviorActionNode *BehaviorActionNode::GetParent() { return parent_node; }

EnumBehaviorTreeStatus BehaviorActionNode::OnInvoke() {
    return EnumBehaviorTreeStatus::eSuccess;
}

void BehaviorActionNode::PushNode(std::shared_ptr<BehaviorActionNode> node) {
    node->parent_node = this;
    node->data_block = data_block;
    child_nodes.push_back(node);
}

void BehaviorActionNode::PopNode() { child_nodes.pop_back(); }

void BehaviorActionNode::Dispose() {
    for (auto &child : child_nodes) {
        child->Dispose();
        child.reset();
    }
};
