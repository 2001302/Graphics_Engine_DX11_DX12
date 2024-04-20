#pragma once

#include "header.h"
#include "Manager.h"

namespace Engine
{

	enum EnumDataBlockType
	{
		eManager = 0,
	};

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
		std::map<EnumDataBlockType, IDataBlock*> DataBlock;
	protected:
		IBehaviorTreeNode* parentNode = 0;
	};

	class BehaviorTreeRootNode : public IBehaviorTreeNode
	{
	public:
		BehaviorTreeRootNode* Excute(IBehaviorTreeNode* node);
		BehaviorTreeRootNode* Sequence();
		BehaviorTreeRootNode* Selector();
		BehaviorTreeRootNode* Close();
	protected:
		std::vector<IBehaviorTreeNode*> childNodes;
	};

	class SequenceNode : public BehaviorTreeRootNode
	{
	public:
		//TODO : need refactoring
		SequenceNode() {};
		SequenceNode(std::map<EnumDataBlockType, IDataBlock*> dataBlock)
		{
			DataBlock = dataBlock;
		};
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
		BehaviorTreeRootNode* Build(std::map<EnumDataBlockType, IDataBlock*> dataBlock)
		{
			m_Tree = new SequenceNode(dataBlock);
			return m_Tree;
		}
		void Run() 
		{
			m_Tree->Invoke();
		}
	private:
		BehaviorTreeRootNode* m_Tree;
	};

	//class Test
	//{
	//public:
	//	void TestFunc()
	//	{
	//		auto builder = new BehaviorTreeBuilder();

	//		builder->Build()
	//			->Sequence()
	//				->Excute(new ActionNode())
	//				->Sequence()
	//					->Excute(new ActionNode())
	//					->Excute(new ActionNode())
	//				->Close()
	//				->Excute(new ActionNode())
	//				->Excute(new ActionNode())
	//			->Close()
	//			->Selector()
	//				->Excute(new ActionNode())
	//				->Excute(new ActionNode())
	//				->Excute(new ActionNode())
	//			->Close();

	//		builder->Run();
	//	};
	//};

}
