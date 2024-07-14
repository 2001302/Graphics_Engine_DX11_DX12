#include "camera.h"
#include "geometry_generator.h"
#include "pipeline_manager.h"
#include "setting_ui.h"

using namespace dx11;

void Camera::Render() {
    float yaw, pitch, roll;
    DirectX::SimpleMath::Matrix rotationMatrix;

    // Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in
    auto rot = rotation * 0.0174532925f;
    pitch = rot.x;
    yaw = rot.y;
    roll = rot.z;

    // Create the rotation matrix from the yaw, pitch, and roll values.
    rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    // Transform the lookAt and up vector by the rotation matrix so the view is
    // correctly rotated at the origin.
    // lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
    upVector = XMVector3TransformCoord(upVector, rotationMatrix);

    // Translate the rotated camera position to the location of the viewer.

    // Finally create the view matrix from the three updated vectors.
    view = XMMatrixLookAtLH(position, lookAtVector, upVector);

    return;
}

EnumBehaviorTreeStatus InitializeCamera::OnInvoke() {
    auto block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the camera object.
    manager->camera = std::make_unique<Camera>();

    manager->camera->lookAtVector =
        DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);
    manager->camera->upVector = DirectX::SimpleMath::Vector3(0.0f, 0.1f, 0.0f);
    manager->camera->position =
        DirectX::SimpleMath::Vector3(0.0f, 0.0f, -10.0f);
    manager->camera->Render();

    // Update the position and rotation of the camera for this scene.
    manager->camera->position =
        DirectX::SimpleMath::Vector3(-10.0f, 1.0f, -10.0f);
    manager->camera->rotation = DirectX::SimpleMath::Vector3(0.0f, 45.0f, 0.0f);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateCamera::OnInvoke() {
    auto managerBlock = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<dx11::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    // Generate the view matrix based on the camera's position.
    manager->camera->Render();

    return EnumBehaviorTreeStatus::eSuccess;
}
