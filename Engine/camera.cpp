#include "camera.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

using namespace dx11;

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

Matrix Camera::GetView() { return view; }

Matrix Camera::GetProjection() {
    return XMMatrixPerspectiveFovLH(XMConvertToRadians(projFovAngleY),
                                    common::Env::Instance().aspect, nearZ,
                                    farZ);
}

Vector3 Camera::GetPosition() { return position; }

Vector3 Camera::GetLookAtVector() { return lookAtVector; }

void Camera::SetPosition(Vector3 pos) { position = pos; }

void Camera::SetLookAtVector(Vector3 lookAt) { lookAtVector = lookAt; }

EnumBehaviorTreeStatus InitializeCamera::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(block);
    assert(manager != nullptr);

    manager->camera = std::make_unique<Camera>();
    manager->camera->Render();

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateCamera::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    manager->camera->Render();

    return EnumBehaviorTreeStatus::eSuccess;
}
