#ifndef _GAMEOBJECTNODE
#define _GAMEOBJECTNODE

#include "graph_node.h"
#include "panel.h"

namespace Engine {
class GameObjectDetailNode : public GraphNode {
  public:
    EnumBehaviorTreeStatus OnInvoke() override;
    EnumBehaviorTreeStatus OnShow() override;

    ImVector<LinkInfo> links_;
    int next_link_Id = 100;
};
class DefaultGraphNode : public GraphNode {
  public:
    EnumBehaviorTreeStatus OnInvoke() override;
    EnumBehaviorTreeStatus OnShow() override;
};

} // namespace Engine
#endif