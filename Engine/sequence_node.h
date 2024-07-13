#ifndef SEQUENCE_NODE
#define SEQUENCE_NODE

#include "behavior_tree.h"

namespace dx11 {

class SequenceNode : public BehaviorActionNode {
  public:
    SequenceNode(){};
    SequenceNode(std::map<EnumDataBlockType, common::IDataBlock *> dataBlock) {
        data_block = dataBlock;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace Engine
#endif
