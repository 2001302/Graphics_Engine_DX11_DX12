#include "parallel_node.h"

using namespace dx11;

EnumBehaviorTreeStatus ParallelNode::OnInvoke() {
    for (auto id : target_ids) {
        this->target_id = id;
        for (auto &child : child_nodes) {
            if (child->Invoke() == EnumBehaviorTreeStatus::eFail)
                return EnumBehaviorTreeStatus::eFail;
        }
    }
    return EnumBehaviorTreeStatus::eSuccess;
}
