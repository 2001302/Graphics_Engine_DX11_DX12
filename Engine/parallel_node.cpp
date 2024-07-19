#include "parallel_node.h"

namespace engine {

EnumBehaviorTreeStatus ParallelNode::OnInvoke() {
    for (auto id : target_objects) {
        this->target_object = id.second;
        for (auto &child : child_nodes) {
            if (child->Invoke() == EnumBehaviorTreeStatus::eFail)
                return EnumBehaviorTreeStatus::eFail;
        }
    }
    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace engine