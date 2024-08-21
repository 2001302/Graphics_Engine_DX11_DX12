#include "animation_node.H"

namespace foundation {

EnumBehaviorTreeStatus AnimationNode::OnReset() {
    is_done = false;
    elapsed_time = 0;
    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace common
