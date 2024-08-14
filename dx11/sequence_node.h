#ifndef SEQUENCE_NODE
#define SEQUENCE_NODE

#include "action_node.h"

namespace common {

class SequenceNode : public BehaviorActionNode {
  public:
    SequenceNode(){};
    SequenceNode(common::IDataBlock * dataBlock) {
        data_block = dataBlock;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace common
#endif
