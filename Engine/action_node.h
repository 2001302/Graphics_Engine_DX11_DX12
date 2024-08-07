#ifndef _BehaviorTree
#define _BehaviorTree

#include "dataBlock.h"
#include "node.h"
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

class BehaviorActionNode : public common::INode {
  public:
    BehaviorActionNode() : parent_node(0), target_object(0), data_block(0){};
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
    common::INode *target_object;
};
} // namespace common
#endif
