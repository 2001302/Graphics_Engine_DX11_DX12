#ifndef _RENDERNODE
#define _RENDERNODE

#include "behavior_tree.h"
#include "conditional_node.h"

namespace Engine {
class InitializeCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};


} // namespace Engine
#endif