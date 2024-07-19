#ifndef CAMERA
#define CAMERA

#include "behavior_tree_builder.h"
#include "direct3d.h"
#include "env.h"
#include "node_ui.h"

namespace dx11 {
class Camera : public common::INodeUi {
  public:
    Camera();
    void Render();

    DirectX::SimpleMath::Matrix GetView();
    DirectX::SimpleMath::Vector3 GetPosition();
    DirectX::SimpleMath::Vector3 GetLookAt();
    DirectX::SimpleMath::Matrix GetProjection();
   
    void SetPosition(DirectX::SimpleMath::Vector3 pos);
    void SetLookAt(DirectX::SimpleMath::Vector3 look);
    
  private:
    DirectX::SimpleMath::Matrix view;
    DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3(-10.0f, 1.0f,
                                                              -10.0f);
    DirectX::SimpleMath::Vector3
        rotation =DirectX::SimpleMath::Vector3(0.0f, 45.0f, 0.0f);

    DirectX::SimpleMath::Vector3 upVector;
    DirectX::SimpleMath::Vector3 lookAtVector;

    float projection_fov_angle_y = 70.0f;
    float near_z = 0.01f;
    float far_z = 100.0f;
};

class InitializeCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace dx11
#endif
