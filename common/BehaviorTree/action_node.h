#ifndef _BehaviorTree
#define _BehaviorTree

#include "../structure/dataBlock.h"
#include "../structure/node.h"
#include <iostream>
#include <map>
#include <vector>

namespace common {
enum EnumBehaviorTreeStatus {
    eSuccess = 0,
    eFail = 1,
    eRunning = 2,
};

class IDisposable {
  public:
    ~IDisposable() { Dispose(); };
    virtual void Dispose(){};
};

class BehaviorActionNode {
  public:
    BehaviorActionNode() : parent_node(0), data_block(0){};
    EnumBehaviorTreeStatus Invoke();
    EnumBehaviorTreeStatus Reset();
    BehaviorActionNode *GetParent();
    void PushNode(std::shared_ptr<BehaviorActionNode> node);
    void PopNode();

  protected:
    virtual EnumBehaviorTreeStatus OnInvoke();
    virtual EnumBehaviorTreeStatus OnReset();
    BehaviorActionNode * parent_node;
    std::vector<std::shared_ptr<BehaviorActionNode>> child_nodes;
    common::IDataBlock * data_block;
};
} // namespace common
#endif
