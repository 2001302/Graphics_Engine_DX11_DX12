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

BehaviorTreeRootNode* BehaviorTreeRootNode::Excute(IBehaviorTreeNode* node)
{
	node->SetParent(this);
	node->DataBlock = DataBlock;
	childNodes.push_back(node);
	return this;
}

BehaviorTreeRootNode* BehaviorTreeRootNode::Sequence()
{
	SequenceNode* node = new SequenceNode();
	node->DataBlock = DataBlock;
	childNodes.push_back(dynamic_cast<IBehaviorTreeNode*>(node));
	return node;
}

BehaviorTreeRootNode* BehaviorTreeRootNode::Selector()
{
	SelectorNode* node = new SelectorNode();
	node->DataBlock = DataBlock;
	childNodes.push_back(dynamic_cast<IBehaviorTreeNode*>(node));
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
