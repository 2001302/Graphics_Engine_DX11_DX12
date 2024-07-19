#include "camera.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

namespace engine {

Camera::Camera() {
    view = DirectX::SimpleMath::Matrix();
    position = DirectX::SimpleMath::Vector3(-10.0f, 1.0f, -10.0f);
    rotation = DirectX::SimpleMath::Vector3(0.0f, 45.0f, 0.0f);
    upVector = DirectX::SimpleMath::Vector3(0.0f, 0.1f, 0.0f);
    lookAtVector = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);

    projection_fov_angle_y = 70.0f;
    near_z = 0.01f;
    far_z = 100.0f;
}
DirectX::SimpleMath::Matrix Camera::GetView() { return view; }
DirectX::SimpleMath::Vector3 Camera::GetPosition() { return position; }
DirectX::SimpleMath::Vector3 Camera::GetLookAt() { return lookAtVector; }
DirectX::SimpleMath::Matrix Camera::GetProjection() {
    return XMMatrixPerspectiveFovLH(XMConvertToRadians(projection_fov_angle_y),
                                    common::Env::Instance().aspect, near_z,
                                    far_z);
}

void Camera::SetPosition(DirectX::SimpleMath::Vector3 pos) { position = pos; }
void Camera::SetLookAt(DirectX::SimpleMath::Vector3 look) {
    lookAtVector = look;
}

void Camera::Render() {
    float yaw, pitch, roll;
    DirectX::SimpleMath::Matrix rotationMatrix;

    auto rot = rotation * 0.0174532925f;
    pitch = rot.x;
    yaw = rot.y;
    roll = rot.z;

    rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
    upVector = XMVector3TransformCoord(upVector, rotationMatrix);
    view = XMMatrixLookAtLH(position, lookAtVector, upVector);

    return;
}

EnumBehaviorTreeStatus InitializeCamera::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<engine::PipelineManager *>(block);
    assert(manager != nullptr);

    manager->camera = std::make_unique<Camera>();
    manager->camera->Render();

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateCamera::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    manager->camera->Render();

    return EnumBehaviorTreeStatus::eSuccess;
}
} // namespace engine