#include "behavior_tree.h"

using namespace Engine;

EnumBehaviorTreeStatus BehaviorActionNode::Invoke() { return OnInvoke(); }

EnumBehaviorTreeStatus BehaviorActionNode::OnInvoke() {
    return EnumBehaviorTreeStatus::eSuccess;
}

void BehaviorActionNode::SetParent(BehaviorActionNode *node) {
    parent_node = node;
}

void BehaviorRootNode::Dispose() {
    for (auto &child : child_nodes) {
        child->Dispose();
        child.reset();
    }
};

BehaviorRootNode *
BehaviorRootNode::Excute(std::shared_ptr<BehaviorActionNode> node) {
    node->SetParent(this);
    node->data_block = data_block;
    child_nodes.emplace_back(node);

    return this;
}

BehaviorRootNode *BehaviorRootNode::Sequence() {
    child_nodes.emplace_back(std::make_shared<SequenceNode>());

    auto node = dynamic_cast<BehaviorRootNode *>(child_nodes.back().get());
    node->data_block = data_block;

    return node;
}

BehaviorRootNode *BehaviorRootNode::Selector() {
    child_nodes.emplace_back(std::make_shared<SelectorNode>());

    auto node = dynamic_cast<BehaviorRootNode *>(child_nodes.back().get());
    node->data_block = data_block;

    return node;
}

BehaviorRootNode *BehaviorRootNode::Close() {
    return dynamic_cast<BehaviorRootNode *>(parent_node);
}

EnumBehaviorTreeStatus SequenceNode::OnInvoke() {
    for (auto &child : child_nodes) {
        if (child->Invoke() == EnumBehaviorTreeStatus::eFail)
            return EnumBehaviorTreeStatus::eFail;
    }
    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus SelectorNode::OnInvoke() {
    for (auto &child : child_nodes) {
        if (child->Invoke() == EnumBehaviorTreeStatus::eSuccess)
            return EnumBehaviorTreeStatus::eSuccess;
    }
    return EnumBehaviorTreeStatus::eSuccess;
}
