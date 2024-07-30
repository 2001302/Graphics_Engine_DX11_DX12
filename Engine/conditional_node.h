#ifndef _CONDITIONALNODE
#define _CONDITIONALNODE

#include "action_node.h"

namespace common {
class ConditionalNode : public BehaviorActionNode {
  public:
    virtual EnumBehaviorTreeStatus CheckCondition();

  private:
    EnumBehaviorTreeStatus OnInvoke() override {
        return EnumBehaviorTreeStatus::eSuccess;
    }
};
} // namespace common
#endif
