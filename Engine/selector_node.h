#ifndef SELECTOR_NODE
#define SELECTOR_NODE

#include "action_node.h"

namespace common {

class SelectorNode : public BehaviorActionNode {
  public:
    SelectorNode(){};
    SelectorNode(std::map<EnumDataBlockType, common::IDataBlock *> dataBlock) {
        data_block = dataBlock;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace common
#endif
