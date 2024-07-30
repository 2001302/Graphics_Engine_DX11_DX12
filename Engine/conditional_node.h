#ifndef _CONDITIONALNODE
#define _CONDITIONALNODE

#include "action_node.h"

namespace common {
class ConditionalNode : public BehaviorActionNode {
  private:
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace common
#endif
