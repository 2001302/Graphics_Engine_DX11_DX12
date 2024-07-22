#ifndef CAMERA
#define CAMERA

#include "behavior_tree_builder.h"
#include "graphics_manager.h"
#include "env.h"
#include "node_ui.h"
#include "graphics_manager.h"

namespace engine {
class Camera : public common::INodeUi {
  public:
    Matrix GetViewRow();
    Matrix GetProjRow();
    Vector3 GetEyePos();

    void UpdateViewDir();
    void UpdateKeyboard(const float dt, bool const keyPressed[256]);
    void UpdateMouse(float mouseNdcX, float mouseNdcY);
    void Rotate(float dx, float dy);
    void MoveForward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);
    void SetEyeWorld(Vector3 pos);

    bool m_useFirstPersonView = false;

  private:
    Vector3 m_position = Vector3(0.275514f, 0.461257f, 0.0855238f);
    Vector3 m_viewDir = Vector3(0.0f, 0.0f, 50.0f);
    Vector3 m_upDir = Vector3(0.0f, 1.0f, 0.0f); // 이번 예제에서는 고정
    Vector3 m_rightDir = Vector3(1.0f, 0.0f, 0.0f);

    // roll, pitch, yaw
    // https://en.wikipedia.org/wiki/Aircraft_principal_axes
    float m_yaw = -0.019635f, m_pitch = -0.120477f;

    float m_speed = 3.0f; // 움직이는 속도

    float m_projFovAngleY = 90.0f;
    float m_nearZ = 0.01f;
    float m_farZ = 100.0f;
    bool m_usePerspectiveProjection = true;
};

class InitializeCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};

class UpdateCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override;
};
} // namespace dx11
#endif
