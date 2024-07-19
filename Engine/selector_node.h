#ifndef SELECTOR_NODE
#define SELECTOR_NODE

#include "behavior_tree.h"

namespace engine {

class SelectorNode : public BehaviorActionNode {
  public:
    SelectorNode(){};
    SelectorNode(std::map<EnumDataBlockType, common::IDataBlock *> dataBlock) {
        data_block = dataBlock;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};

} // namespace dx11
#endif
