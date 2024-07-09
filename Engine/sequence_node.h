#ifndef SEQUENCE_NODE
#define SEQUENCE_NODE

#include "behavior_tree.h"

namespace Engine {

class SequenceNode : public BehaviorActionNode {
  public:
    SequenceNode(){};
    SequenceNode(std::map<EnumDataBlockType, IDataBlock *> dataBlock) {
        data_block = dataBlock;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace Engine
#endif
