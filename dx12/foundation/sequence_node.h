#ifndef SEQUENCE_NODE
#define SEQUENCE_NODE

#include "action_node.h"

namespace foundation {

class SequenceNode : public BehaviorActionNode {
  public:
    SequenceNode(){};
    SequenceNode(foundation::IDataBlock * dataBlock) {
        data_block = dataBlock;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace common
#endif
