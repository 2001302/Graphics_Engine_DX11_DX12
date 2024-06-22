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

class BehaviorActionNode : public IDisposable {
  public:
    void SetParent(BehaviorActionNode *node);
    std::map<EnumDataBlockType, IDataBlock *> data_block;
    EnumBehaviorTreeStatus Invoke();

  protected:
    virtual EnumBehaviorTreeStatus OnInvoke();
    BehaviorActionNode *parent_node = 0;
};

class BehaviorRootNode : public BehaviorActionNode {
  public:
    void Dispose() override;
    BehaviorRootNode *Excute(std::shared_ptr<BehaviorActionNode> node);
    BehaviorRootNode *Sequence();
    BehaviorRootNode *Selector();
    BehaviorRootNode *Close();

  protected:
    std::vector<std::shared_ptr<BehaviorActionNode>> child_nodes;
};

class SequenceNode : public BehaviorRootNode {
  public:
    SequenceNode(){};
    SequenceNode(std::map<EnumDataBlockType, IDataBlock *> dataBlock) {
        data_block = dataBlock;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};

class SelectorNode : public BehaviorRootNode {
  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace Engine
#endif