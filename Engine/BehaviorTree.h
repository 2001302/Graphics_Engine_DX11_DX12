#pragma once

#include "header.h"

namespace BehaviorTree
{
	enum EnumBehaviorTreeStatus
	{
		eSuccess = 0,
		eFail = 1,
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

	protected:
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

		SequenceNode* Sequence()
		{
			SequenceNode* node = new SequenceNode();
			childNodes.push_back(dynamic_cast<IBehaviorTreeNode*>(node));
			return node;
		}

		SelectorNode* Selector()
		{
			SelectorNode* node = new SelectorNode();
			childNodes.push_back(dynamic_cast<IBehaviorTreeNode*>(node));
			return node;
		}

		BehaviorTreeRootNode* Close()
		{
			return dynamic_cast<BehaviorTreeRootNode*>(parentNode);
		}

	protected:
		std::vector<IBehaviorTreeNode*> childNodes;
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

	class BehaviorTreeBuilder
	{
	public:
		BehaviorTreeRootNode* Build()
		{
			m_Tree = new BehaviorTreeRootNode();
			return m_Tree;
		}
		void Run() 
		{
			m_Tree->Invoke();
		}
	private:
		BehaviorTreeRootNode* m_Tree;
	};


	class Test
	{
	public:
		void TestFunc()
		{
			auto builder = new BehaviorTreeBuilder();

			builder->Build()
				->Sequence()
					->Excute(new ActionNode())
					->Sequence()
						->Excute(new ActionNode())
						->Excute(new ActionNode())
					->Close()
					->Excute(new ActionNode())
					->Excute(new ActionNode())
				->Close()
				->Selector()
					->Excute(new ActionNode())
					->Excute(new ActionNode())
					->Excute(new ActionNode())
				->Close();

			builder->Run();
		};
	};
}
