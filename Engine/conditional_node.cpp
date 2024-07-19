#include "conditional_node.h"

namespace engine {

// check condition
EnumBehaviorTreeStatus ConditionalNode::CheckCondition() {
    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace engine