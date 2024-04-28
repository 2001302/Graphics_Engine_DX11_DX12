#pragma once

#include "CommonModel.h"

namespace Engine
{

	enum EnumDataBlockType
	{
		eManager = 0,
		eViewingPoint=1,
	};

	enum EnumBehaviorTreeStatus
	{
		eSuccess = 0,
		eFail = 1,
	};

	class IBehaviorTreeNode : public IDisposable
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
		void Dispose() override 
		{
			for (auto& child : childNodes)
			{
				child->Dispose();
				delete child;
				child = 0;
			}
		};
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
		std::shared_ptr<BehaviorTreeRootNode> Build(std::map<EnumDataBlockType, IDataBlock*> dataBlock)
		{
			m_Tree = make_shared<SequenceNode>(dataBlock);
			return m_Tree;
		}
		void Run()
		{
			m_Tree->Invoke();
			m_Tree->Dispose();
			m_Tree.reset();
		}
	private:
		std::shared_ptr<BehaviorTreeRootNode> m_Tree;
	};
}
