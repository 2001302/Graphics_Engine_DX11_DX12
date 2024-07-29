#ifndef SEQUENCE_NODE
#define SEQUENCE_NODE

#include "action_node.h"

namespace engine {

class SequenceNode : public BehaviorActionNode {
  public:
    SequenceNode(){};
    SequenceNode(std::map<EnumDataBlockType, common::IDataBlock *> dataBlock) {
        data_block = dataBlock;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace dx11
#endif
