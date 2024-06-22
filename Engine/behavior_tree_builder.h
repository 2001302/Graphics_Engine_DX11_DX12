#ifndef BEHAVIORTREEBUILDER
#define BEHAVIORTREEBUILDER

#include "common_struct.h"
#include "behavior_tree.h"

namespace Engine {

class BehaviorTreeBuilder {
  public:
    std::shared_ptr<BehaviorRootNode>
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
    std::shared_ptr<BehaviorRootNode> tree;
};
} // namespace Engine
#endif