#include "camera.h"
#include "../util/geometry_generator.h"
#include "../component/mesh_renderer.h"
#include "render_target_object.h"
#include <gui/setting_ui.h>

namespace graphics {

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

    yaw = -0.019635f;
    pitch = -0.120477f;
}

Matrix Camera::GetView() { return view; }

Vector3 Camera::GetPosition() { return position; }

Vector3 Camera::GetLookAt() { return lookAtVector; }

Matrix Camera::GetProjection() {
    return DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(projection_fov_angle_y),
        (float)common::env::screen_width / (float)common::env::screen_height,
        near_z, far_z);
}

void Camera::SetPosition(Vector3 pos) { position = pos; }
void Camera::SetLookAt(Vector3 look) { lookAtVector = look; }

void Camera::Initialize() {
    MeshData sphere = GeometryGenerator::MakeSphere(0.01f, 10, 10);

    auto renderer = std::make_shared<MeshRenderer>(std::vector{sphere});

    renderer->UpdateWorldRow(Matrix::CreateTranslation(lookAtVector));
    renderer->material_consts.GetCpu().albedoFactor = Vector3(0.0f);
    renderer->material_consts.GetCpu().emissionFactor =
        Vector3(1.0f, 0.0f, 0.0f);
    renderer->cast_shadow = false;

    look_at_target = std::make_shared<common::Model>();
    look_at_target->TryAdd(renderer);
}

void Camera::Update() {
    Matrix rotationMatrix =
        DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f);
    auto up = XMVector3TransformCoord(upVector, rotationMatrix);
    view = XMMatrixLookAtLH(position, lookAtVector, up);

    MeshRenderer *renderer = nullptr;
    if (look_at_target->TryGet(renderer)) {
        renderer->UpdateWorldRow(Matrix::CreateScale(1.0f) *
                                 Matrix::CreateTranslation(lookAtVector));
        renderer->UpdateConstantBuffers();
    }
}

void Camera::Draw() {
    MeshRenderer *renderer = nullptr;
    if (look_at_target->TryGet(renderer)) {
        renderer->Render();
    }
}
} // namespace graphics