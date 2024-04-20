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
		virtual EnumBehaviorTreeStatus Invoke();
		void SetParent(IBehaviorTreeNode* node);

	protected:
		IBehaviorTreeNode* parentNode;
	};

	class BehaviorTreeRootNode : public IBehaviorTreeNode
	{
	protected:
		std::vector<IBehaviorTreeNode*> childNodes;
	public:
		BehaviorTreeRootNode* Excute(IBehaviorTreeNode* node);
		BehaviorTreeRootNode* Sequence();
		BehaviorTreeRootNode* Selector();
		BehaviorTreeRootNode* Close();
	};

	class SequenceNode : public BehaviorTreeRootNode
	{
	public:
		EnumBehaviorTreeStatus Invoke() override;
	};

	class SelectorNode : public BehaviorTreeRootNode
	{
	public:
		EnumBehaviorTreeStatus Invoke() override;
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
