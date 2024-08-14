#include "behavior_tree_builder.h"

namespace common {

BehaviorTreeBuilder *
BehaviorTreeBuilder::Excute(std::shared_ptr<BehaviorActionNode> node) {
    current->PushNode(node);
    return this;
}

BehaviorTreeBuilder *BehaviorTreeBuilder::Sequence() {
    auto node = std::make_shared<SequenceNode>();
    current->PushNode(node);
    current = node.get();

    return this;
}

BehaviorTreeBuilder *BehaviorTreeBuilder::Selector() {
    auto node = std::make_shared<SelectorNode>();
    current->PushNode(node);
    current = node.get();

    return this;
}

BehaviorTreeBuilder *
BehaviorTreeBuilder::Conditional(std::shared_ptr<ConditionalNode> node) {
    current->PushNode(node);
    current = node.get();

    return this;
}

BehaviorTreeBuilder *BehaviorTreeBuilder::Close() {
    current = current->GetParent();
    return this;
}

BehaviorTreeBuilder *
BehaviorTreeBuilder::Loop(std::map<int, common::INode *> target_objects) {
    auto node = std::make_shared<ParallelNode>(target_objects);
    current->PushNode(node);
    current = node.get();

    return this;
}
} // namespace common