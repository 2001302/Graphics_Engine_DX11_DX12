#ifndef _GAMEOBJECTNODE
#define _GAMEOBJECTNODE

#include "graph_node.h"

namespace Engine {
class GameObjectDetailNode : public GraphNode {
    EnumBehaviorTreeStatus Invoke() override;
    EnumBehaviorTreeStatus Render() override;
};

} // namespace Engine
#endif