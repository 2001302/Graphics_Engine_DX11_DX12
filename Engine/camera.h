#ifndef CAMERA
#define CAMERA

#include "behavior_tree_builder.h"
#include "node_ui.h"
#include "direct3d.h"

namespace dx11 {
class Camera : public common::INodeUi {
  public:
    void Render();

    DirectX::SimpleMath::Matrix view;
    DirectX::SimpleMath::Vector3 position;
    DirectX::SimpleMath::Vector3 rotation;
    
    DirectX::SimpleMath::Vector3 upVector;
    DirectX::SimpleMath::Vector3 lookAtVector;
};

class InitializeCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace dx11
#endif
