#ifndef _BehaviorTree
#define _BehaviorTree

#include "common_struct.h"
#include "dataBlock.h"

namespace dx11 {
enum EnumDataBlockType {
    eManager = 0,
    eGui = 1,
};

enum EnumBehaviorTreeStatus {
    eSuccess = 0,
    eFail = 1,
};

enum EnumConditionalStatus {
    ePass = 0,
    eSkip = 1,
};

class IDisposable {
  public:
    ~IDisposable() { Dispose(); };
    virtual void Dispose(){};
};

class BehaviorActionNode : public IDisposable {
  public:
    BehaviorActionNode() : parent_node(0), target_id(0){};
    EnumBehaviorTreeStatus Invoke();
    BehaviorActionNode *GetParent();
    void Dispose() override;
    void PushNode(std::shared_ptr<BehaviorActionNode> node);
    void PopNode();

  protected:
    virtual EnumBehaviorTreeStatus OnInvoke();
    BehaviorActionNode *parent_node;
    std::vector<std::shared_ptr<BehaviorActionNode>> child_nodes;
    std::map<EnumDataBlockType, common::IDataBlock *> data_block;
    int target_id;
};
} // namespace Engine
#endif
