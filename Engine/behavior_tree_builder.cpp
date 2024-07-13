#include "behavior_tree_builder.h"

using namespace dx11;

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
    if (node->CheckCondition() == EnumBehaviorTreeStatus::eSuccess) {
        conditional_status = EnumConditionalStatus::ePass;
    }
    else {
		conditional_status = EnumConditionalStatus::eSkip;
	}
    current->PopNode();
    return this;
}

BehaviorTreeBuilder *BehaviorTreeBuilder::End() {
    if (conditional_status == EnumConditionalStatus::eSkip) {
        current->PopNode();
    }
    return this;
}

BehaviorTreeBuilder *BehaviorTreeBuilder::Close() {
    current = current->GetParent();
    return this;
}

BehaviorTreeBuilder *
BehaviorTreeBuilder::Parallel(std::vector<int> target_ids) {
    auto node = std::make_shared<ParallelNode>(target_ids);
    current->PushNode(node);
    current = node.get();

    return this;
}