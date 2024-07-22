#include "camera.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

namespace engine {

Matrix Camera::GetViewRow() {
    return Matrix::CreateTranslation(-this->m_position) *
           Matrix::CreateRotationY(-this->m_yaw) *
           Matrix::CreateRotationX(this->m_pitch);
}

Vector3 Camera::GetEyePos() { return m_position; }

void Camera::UpdateViewDir() {
    // 이동할 때 기준이 되는 정면/오른쪽 방향 계산
    m_viewDir = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f),
                                   Matrix::CreateRotationY(this->m_yaw));
    m_rightDir = m_upDir.Cross(m_viewDir);
}

void Camera::UpdateKeyboard(const float dt, bool const keyPressed[256]) {
    if (m_useFirstPersonView) {
        if (keyPressed['W'])
            MoveForward(dt);
        if (keyPressed['S'])
            MoveForward(-dt);
        if (keyPressed['D'])
            MoveRight(dt);
        if (keyPressed['A'])
            MoveRight(-dt);
        if (keyPressed['E'])
            MoveUp(dt);
        if (keyPressed['Q'])
            MoveUp(-dt);
    }
}

void Camera::UpdateMouse(float mouseNdcX, float mouseNdcY) {
    if (m_useFirstPersonView) {
        // 얼마나 회전할지 계산
        m_yaw = mouseNdcX * DirectX::XM_2PI;      // 좌우 360도
        m_pitch = mouseNdcY * DirectX::XM_PIDIV2; // 위 아래 90도

        UpdateViewDir();
    }
}

void Camera::Rotate(float dx, float dy) {
    m_yaw = m_yaw + dx * DirectX::XM_2PI;        // 좌우 360도
    m_pitch = m_pitch + dy * DirectX::XM_PIDIV2; // 위 아래 90도

    UpdateViewDir();
}

void Camera::MoveForward(float dt) {
    // 이동후의_위치 = 현재_위치 + 이동방향 * 속도 * 시간차이;
    m_position += m_viewDir * m_speed * dt;
}

void Camera::MoveUp(float dt) {
    // 이동후의_위치 = 현재_위치 + 이동방향 * 속도 * 시간차이;
    m_position += m_upDir * m_speed * dt;
}

void Camera::MoveRight(float dt) { m_position += m_rightDir * m_speed * dt; }

void Camera::SetEyeWorld(Vector3 pos) { m_position = pos; }

Matrix Camera::GetProjRow() {
    return m_usePerspectiveProjection
               ? XMMatrixPerspectiveFovLH(XMConvertToRadians(m_projFovAngleY),
                                          common::Env::Instance().GetAspect(),
                                          m_nearZ, m_farZ)
               : XMMatrixOrthographicOffCenterLH(
                     -common::Env::Instance().GetAspect(),
                     common::Env::Instance().GetAspect(), -1.0f, 1.0f, m_nearZ,
                     m_farZ);
}

EnumBehaviorTreeStatus InitializeCamera::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<engine::PipelineManager *>(block);
    assert(manager != nullptr);

    manager->camera = std::make_unique<Camera>();

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateCamera::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    manager->m_globalConstsCPU.eyeWorld = manager->camera->GetEyePos();
    manager->m_globalConstsCPU.view = manager->camera->GetViewRow().Transpose();
    manager->m_globalConstsCPU.proj = manager->camera->GetProjRow().Transpose();
    manager->m_globalConstsCPU.invProj =
        manager->m_globalConstsCPU.proj.Invert().Transpose();
    manager->m_globalConstsCPU.viewProj =
        (manager->m_globalConstsCPU.view * manager->m_globalConstsCPU.proj)
            .Transpose();

    manager->m_globalConstsCPU.textureToDraw = 0;
    manager->m_globalConstsCPU.envLodBias = 0.1f;
    manager->m_globalConstsCPU.lodBias = 2.1f;

    auto device = GraphicsManager::Instance().device;
    auto context = GraphicsManager::Instance().device_context;

    GraphicsUtil::UpdateBuffer(device, context, manager->m_globalConstsCPU,
                               manager->m_globalConstsGPU);

    // GraphicsManager::Instance().SetGlobalConsts(manager->m_globalConstsGPU);

    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace engine