#ifndef _CONDITIONALNODE
#define _CONDITIONALNODE

#include "behavior_tree.h"

namespace Engine {
class ConditionalNode : public BehaviorActionNode {
  public:
    virtual EnumBehaviorTreeStatus CheckCondition();
    void SetId(int target) { this->target_id_ = target; }
  private:
    EnumBehaviorTreeStatus OnInvoke() override {
        if (parent_node)
            this->target_id_ = parent_node->target_id();
        return EnumBehaviorTreeStatus::eSuccess;
    }
};
} // namespace Engine
#endif
