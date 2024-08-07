#ifndef ANMATION_NODE
#define ANMATION_NODE

#include "action_node.h"

namespace common {

class AnimationNode : public BehaviorActionNode {
  public:
    AnimationNode() : is_done(false), frame_count(0){};
  protected:
    EnumBehaviorTreeStatus OnReset() override;
    bool is_done;
    int frame_count;
};

} // namespace common
#endif
