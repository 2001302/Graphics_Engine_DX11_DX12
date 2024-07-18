#ifndef CAMERA
#define CAMERA

#include "behavior_tree_builder.h"
#include "graphics_manager.h"
#include "node_ui.h"

namespace dx11 {
class Camera : public common::INodeUi {
  public:
    Camera() {
        projFovAngleY = 90.0f;
        nearZ = 0.01f;
        farZ = 100.0f;

        lookAtVector = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
        upVector = DirectX::SimpleMath::Vector3(0.0f, 0.1f, 0.0f);
        position = DirectX::SimpleMath::Vector3(-10.0f, 1.0f, -10.0f);
        rotation = DirectX::SimpleMath::Vector3(0.0f, 45.0f, 0.0f);
    }
    void Render();
    Matrix GetView();
    Matrix GetProjection();
    Vector3 GetPosition();
    Vector3 GetLookAtVector();
    void SetPosition(Vector3 pos);
    void SetLookAtVector(Vector3 lookAt);

  private:
    DirectX::SimpleMath::Matrix view;
    DirectX::SimpleMath::Vector3 position;
    DirectX::SimpleMath::Vector3 rotation;

    DirectX::SimpleMath::Vector3 upVector;
    DirectX::SimpleMath::Vector3 lookAtVector;

    float projFovAngleY;
    float nearZ;
    float farZ;
};

class InitializeCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace dx11
#endif
