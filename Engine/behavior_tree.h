#ifndef _BehaviorTree
#define _BehaviorTree

#include "common_struct.h"
#include "dataBlock.h"

namespace Engine {
enum EnumDataBlockType {
    eManager = 0,
    eEnv = 1,
    eGui = 2,
};

enum EnumBehaviorTreeStatus {
    eSuccess = 0,
    eFail = 1,
};

/// <summary>
/// Dispose Pattern
/// </summary>
class IDisposable {
  public:
    ~IDisposable() { Dispose(); };
    virtual void Dispose(){};
};

class IBehaviorTreeNode : public IDisposable {
  public:
    virtual EnumBehaviorTreeStatus Invoke();
    void SetParent(IBehaviorTreeNode *node);
    std::map<EnumDataBlockType, IDataBlock *> DataBlock;

  protected:
    IBehaviorTreeNode *parentNode = 0;
};

class BehaviorTreeRootNode : public IBehaviorTreeNode {
  public:
    void Dispose() override {
        for (auto &child : childNodes) {
            child->Dispose();
            child.reset();
        }
    };
    BehaviorTreeRootNode *Excute(std::shared_ptr<IBehaviorTreeNode> node);
    BehaviorTreeRootNode *Sequence();
    BehaviorTreeRootNode *Selector();
    BehaviorTreeRootNode *Close();

  protected:
    std::vector<std::shared_ptr<IBehaviorTreeNode>> childNodes;
};

class SequenceNode : public BehaviorTreeRootNode {
  public:
    SequenceNode(){};
    SequenceNode(std::map<EnumDataBlockType, IDataBlock *> dataBlock) {
        DataBlock = dataBlock;
    };
    EnumBehaviorTreeStatus Invoke() override;
};

class SelectorNode : public BehaviorTreeRootNode {
  public:
    EnumBehaviorTreeStatus Invoke() override;
};

class ActionNode : public IBehaviorTreeNode {};

class BehaviorTreeBuilder {
  public:
    std::shared_ptr<BehaviorTreeRootNode>
    Build(std::map<EnumDataBlockType, IDataBlock *> dataBlock) {
        tree = std::make_shared<SequenceNode>(dataBlock);
        return tree;
    }
    void Run() {
        tree->Invoke();
        tree->Dispose();
        tree.reset();
    }

  private:
    std::shared_ptr<BehaviorTreeRootNode> tree;
};
} // namespace Engine
#endif