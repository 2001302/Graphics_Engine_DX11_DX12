#ifndef PARALLEL_NODE
#define PARALLEL_NODE

#include "behavior_tree.h"

namespace dx11 {

class ParallelNode : public BehaviorActionNode {
  public:
    ParallelNode(std::map<int, common::INodeUi *> target_objects) {
        this->target_objects = target_objects;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
    std::map<int, common::INodeUi*> target_objects;
};

} // namespace dx11
#endif
