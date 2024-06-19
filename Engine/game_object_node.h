#ifndef _GAMEOBJECTNODE
#define _GAMEOBJECTNODE

#include "graph_node.h"
#include "panel.h"

namespace Engine {
class GameObjectDetailNode : public GraphNode {
  public:
    EnumBehaviorTreeStatus OnInvoke() override;
    EnumBehaviorTreeStatus OnShow() override;
    // List of live links. It is dynamic unless youwant to create read-only view
    // over nodes.
    ImVector<LinkInfo> links_;
    // Counter to help generate link ids. In real application this will probably
    // based on pointer to user data structure.
    int next_link_Id = 100;
};

} // namespace Engine
#endif