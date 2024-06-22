#ifndef SELECTOR_NODE
#define SELECTOR_NODE

#include "behavior_tree.h"

namespace Engine {

class SelectorNode : public BehaviorActionNode {
  public:
    SelectorNode(){};
    SelectorNode(std::map<EnumDataBlockType, IDataBlock *> dataBlock) {
        data_block = dataBlock;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace Engine
#endif