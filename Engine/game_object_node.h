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

    bool use_blinn_phong = false;
    bool use_texture = false;

    bool use_perspective_projection = true;
    DirectX::SimpleMath::Vector3 translation =
        DirectX::SimpleMath::Vector3(0.0f);
    DirectX::SimpleMath::Vector3 rotation =
        DirectX::SimpleMath::Vector3(0.0f);
    DirectX::SimpleMath::Vector3 scaling =
        DirectX::SimpleMath::Vector3(1.0f);

    float shininess = 1.0f;
    float diffuse = 1.0f;
    float specular = 1.0f;
};
class DefaultGraphNode : public GraphNode {
  public:
    EnumBehaviorTreeStatus OnInvoke() override;
    EnumBehaviorTreeStatus OnShow() override;
};

} // namespace Engine
#endif