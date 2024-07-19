#ifndef _CONDITIONALNODE
#define _CONDITIONALNODE

#include "behavior_tree.h"

namespace engine {
class ConditionalNode : public BehaviorActionNode {
  public:
    virtual EnumBehaviorTreeStatus CheckCondition();

  private:
    EnumBehaviorTreeStatus OnInvoke() override {
        return EnumBehaviorTreeStatus::eSuccess;
    }
};
} // namespace dx11
#endif
