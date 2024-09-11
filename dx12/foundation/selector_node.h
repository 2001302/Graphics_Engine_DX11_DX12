#ifndef SELECTOR_NODE
#define SELECTOR_NODE

#include "action_node.h"

namespace foundation {

class SelectorNode : public BehaviorActionNode {
  public:
    SelectorNode(){};
    SelectorNode(foundation::IDataBlock * dataBlock) {
        data_block = dataBlock;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace common
#endif
