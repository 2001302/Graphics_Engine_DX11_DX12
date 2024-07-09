#ifndef _BehaviorTree
#define _BehaviorTree

#include "common_struct.h"
#include "dataBlock.h"

namespace Engine {
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
    EnumBehaviorTreeStatus Invoke();
    BehaviorActionNode *GetParent();
    void Dispose() override;
    void PushNode(std::shared_ptr<BehaviorActionNode> node);
    void PopNode();

  protected:
    virtual EnumBehaviorTreeStatus OnInvoke();
    BehaviorActionNode *parent_node;
    std::vector<std::shared_ptr<BehaviorActionNode>> child_nodes;
    std::map<EnumDataBlockType, IDataBlock *> data_block;
};
} // namespace Engine
#endif
