#ifndef PARALLEL_NODE
#define PARALLEL_NODE

#include "behavior_tree.h"

namespace Engine {

class ParallelNode : public BehaviorActionNode {
  public:
    ParallelNode(std::vector<int> target_ids) {
        this->target_ids = target_ids;
    };

  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
    std::vector<int> target_ids;
};

} // namespace Engine
#endif
