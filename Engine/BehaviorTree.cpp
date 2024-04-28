#include"BehaviorTree.h"

using namespace Engine;

EnumBehaviorTreeStatus IBehaviorTreeNode::Invoke()
{
	return EnumBehaviorTreeStatus::eSuccess;
}

void IBehaviorTreeNode::SetParent(IBehaviorTreeNode* node)
{
	parentNode = node;
}

BehaviorTreeRootNode* BehaviorTreeRootNode::Excute(std::shared_ptr<IBehaviorTreeNode> node)
{
	node->SetParent(this);
	node->DataBlock = DataBlock;
	childNodes.emplace_back(node);

	return this;
}

BehaviorTreeRootNode* BehaviorTreeRootNode::Sequence()
{
	childNodes.emplace_back(std::make_shared<SequenceNode>());

	auto node = dynamic_cast<BehaviorTreeRootNode*>(childNodes.back().get());
	node->DataBlock = DataBlock;

	return node;
}

BehaviorTreeRootNode* BehaviorTreeRootNode::Selector()
{
	childNodes.emplace_back(std::make_shared<SelectorNode>());

	auto node = dynamic_cast<BehaviorTreeRootNode*>(childNodes.back().get());
	node->DataBlock = DataBlock;

	return node;
}

BehaviorTreeRootNode* BehaviorTreeRootNode::Close()
{
	return dynamic_cast<BehaviorTreeRootNode*>(parentNode);
}

EnumBehaviorTreeStatus SequenceNode::Invoke()
{
	for (auto& child : childNodes)
	{
		if (child->Invoke() == EnumBehaviorTreeStatus::eFail)
			return EnumBehaviorTreeStatus::eFail;
	}
	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus SelectorNode::Invoke() 
{
	for (auto& child : childNodes)
	{
		if (child->Invoke() == EnumBehaviorTreeStatus::eSuccess)
			return EnumBehaviorTreeStatus::eSuccess;
	}
	return EnumBehaviorTreeStatus::eSuccess;
}
