#ifndef CAMERA
#define CAMERA

#include "common_struct.h"
#include "behavior_tree_builder.h"

namespace Engine {
class Camera {
  public:
    void Render();

    DirectX::SimpleMath::Matrix view;
    DirectX::SimpleMath::Vector3 position;
    DirectX::SimpleMath::Vector3 rotation;
};

class InitializeCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace Engine
#endif