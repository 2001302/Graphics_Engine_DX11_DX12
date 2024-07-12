#include "conditional_node.h"

using namespace Engine;

//check condition
EnumBehaviorTreeStatus ConditionalNode::CheckCondition() {
    if (parent_node)
        this->target_id_ = parent_node->target_id();
    return EnumBehaviorTreeStatus::eSuccess;
}
