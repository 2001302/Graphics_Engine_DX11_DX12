#pragma once

#include "header.h"

enum EnumBehaviorTreeStatus
{
	eSuccess = 0,
	eFail = 1,
};

class BehaviorTreeBuilder
{
private:
	std::vector<IBehaviorTreeNode*> Nodes;

public:
	SequenceNode* Sequence()
	{
		auto sequence = new SequenceNode();
		sequence->SetBuilder(this);
		Nodes.push_back(dynamic_cast<IBehaviorTreeNode*>(sequence));
		return sequence;
	}
	SelectorNode* Selector()
	{
		auto selector = new SelectorNode();
		selector->SetBuilder(this);
		Nodes.push_back(dynamic_cast<IBehaviorTreeNode*>(selector));
		return selector;
	}
};

class IBehaviorTreeNode 
{
public:
	virtual EnumBehaviorTreeStatus Invoke()
	{
		return EnumBehaviorTreeStatus::eSuccess;
	}

	void SetParent(IBehaviorTreeNode* node)
	{
		parentNode = node;
	}

private:
	IBehaviorTreeNode* parentNode;
};

class BehaviorTreeRootNode : public IBehaviorTreeNode
{
public:
	BehaviorTreeRootNode* Excute(IBehaviorTreeNode* node)
	{
		node->SetParent(this);
		childNodes.push_back(node);
		return this;
	}
	BehaviorTreeBuilder* Close() 
	{
		return builder;
	}
	void SetBuilder(BehaviorTreeBuilder* other)
	{
		builder = other;
	}

protected:
	std::vector<IBehaviorTreeNode*> childNodes;
	BehaviorTreeBuilder* builder;
};

class SequenceNode : public BehaviorTreeRootNode
{
	EnumBehaviorTreeStatus Invoke() override
	{
		for (auto& child : childNodes)
		{
			if (child->Invoke() == EnumBehaviorTreeStatus::eFail)
				return EnumBehaviorTreeStatus::eFail;
		}
		return EnumBehaviorTreeStatus::eSuccess;
	}
};

class SelectorNode : public BehaviorTreeRootNode
{
public:
	EnumBehaviorTreeStatus Invoke() override
	{
		for (auto& child : childNodes)
		{
			if (child->Invoke() == EnumBehaviorTreeStatus::eSuccess)
				return EnumBehaviorTreeStatus::eSuccess;
		}
		return EnumBehaviorTreeStatus::eSuccess;
	}
};

class ActionNode : public IBehaviorTreeNode
{

};

class Test
{
public: 
	void TestFunc()
	{
		auto builder = new BehaviorTreeBuilder();

		builder
			->Sequence()
				->Excute(new ActionNode())
				->Excute(new ActionNode())
				->Excute(new ActionNode())
			->Close()
			->Selector()
				->Excute(new ActionNode())
				->Excute(new ActionNode())
				->Excute(new ActionNode())
			->Close();
			
	};
};