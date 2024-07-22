#include "camera.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

namespace engine {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

Camera::Camera() {
    view = Matrix();
    position = Vector3(-5.0f, 5.0f, -5.0f);
    upVector = Vector3(0.0f, 0.1f, 0.0f);
    lookAtVector = Vector3(0.0f, 0.0f, 0.0f);

    projection_fov_angle_y = 70.0f;
    near_z = 0.01f;
    far_z = 100.0f;

    m_yaw = -0.019635f;
    m_pitch = -0.120477f;
}

Matrix Camera::GetView() { return view; }

Vector3 Camera::GetPosition() { return position; }

Vector3 Camera::GetLookAt() { return lookAtVector; }

Matrix Camera::GetProjection() {
    return DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(projection_fov_angle_y),
        common::Env::Instance().GetAspect(), near_z, far_z);
}

void Camera::SetPosition(Vector3 pos) { position = pos; }
void Camera::SetLookAt(Vector3 look) { lookAtVector = look; }

void Camera::Update() {
    Matrix rotationMatrix =
        DirectX::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);
    auto up = XMVector3TransformCoord(upVector, rotationMatrix);
    view = XMMatrixLookAtLH(position, lookAtVector, up);

    return;
}
} // namespace engine