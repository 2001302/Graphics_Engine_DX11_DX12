#ifndef _GAMEOBJECTNODE
#define _GAMEOBJECTNODE

#include "graph_node.h"

namespace Engine {
class GameObjectDetailNode : public GraphNode {
    EnumBehaviorTreeStatus OnInvoke() override;
    EnumBehaviorTreeStatus OnShow() override;
};

} // namespace Engine
#endif