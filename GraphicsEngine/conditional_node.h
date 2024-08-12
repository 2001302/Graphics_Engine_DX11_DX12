#ifndef _CONDITIONALNODE
#define _CONDITIONALNODE

#include "action_node.h"

namespace common {
class ConditionalNode : public BehaviorActionNode {
  protected:
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace common
#endif
