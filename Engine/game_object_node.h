#ifndef _GAMEOBJECTNODE
#define _GAMEOBJECTNODE

#ifdef _MSC_VER
#define portable_strcpy strcpy_s
#define portable_sprintf sprintf_s
#else
#define portable_strcpy strcpy
#define portable_sprintf sprintf
#endif

#include "base_gui.h"
#include "graph.h"

namespace Engine {
struct LinkInfo {
    ed::LinkId Id;
    ed::PinId InputId;
    ed::PinId OutputId;
};

class GameObjectDetailNode : public GraphNode {
  public:
    EnumBehaviorTreeStatus OnInvoke() override;
    EnumBehaviorTreeStatus OnShow() override;

    ImVector<LinkInfo> links_;
    int next_link_Id = 100;

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

} // namespace Engine
#endif