#include "conditional_node.h"

namespace common {

// check condition
EnumBehaviorTreeStatus ConditionalNode::CheckCondition() {
    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace common