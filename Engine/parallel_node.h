#ifndef PARALLEL_NODE
#define PARALLEL_NODE

#include "action_node.h"

namespace engine {

class ParallelNode : public BehaviorActionNode {
  public:
    ParallelNode(std::map<int, common::INode *> target_objects) {
        this->target_objects = target_objects;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
    std::map<int, common::INode*> target_objects;
};

} // namespace dx11
#endif
