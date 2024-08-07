#include "animation_node.H"

namespace common {

EnumBehaviorTreeStatus AnimationNode::OnReset() {
    is_done = false;
    frame_count = 0;
    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace common