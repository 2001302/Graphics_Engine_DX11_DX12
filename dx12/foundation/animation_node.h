#ifndef ANMATION_NODE
#define ANMATION_NODE

#include "action_node.h"

namespace foundation {

class AnimationNode : public BehaviorActionNode {
  public:
    AnimationNode() : is_done(false), elapsed_time(0){};
  protected:
    EnumBehaviorTreeStatus OnReset() override;
    bool is_done;
    float elapsed_time;
};

} // namespace common
#endif
