#include "camera.h"
#include "geometry_generator.h"
#include "setting_ui.h"
#include "pipeline_manager.h"

using namespace Engine;

void Camera::Render() {
    DirectX::SimpleMath::Vector3 upVector, positionVector, lookAtVector;
    // XMVECTOR upVector, positionVector, lookAtVector;
    float yaw, pitch, roll;
    DirectX::SimpleMath::Matrix rotationMatrix;

    // Setup the vector that points upwards.
    upVector = DirectX::SimpleMath::Vector3(0.0f, 0.1f, 0.0f);

    // Setup the position of the camera in the world.
    positionVector = position;

    // Setup where the camera is looking by default.
    lookAtVector = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 0.0f);

    // Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in
    // radians.
    auto rot = rotation * 0.0174532925f;
    pitch = rot.x;
    yaw = rot.y;
    roll = rot.z;

    // Create the rotation matrix from the yaw, pitch, and roll values.
    rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

    // Transform the lookAt and up vector by the rotation matrix so the view is
    // correctly rotated at the origin.
    // lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
    // //TODO : need SetLookAt
    upVector = XMVector3TransformCoord(upVector, rotationMatrix);

    // Translate the rotated camera position to the location of the viewer.
    // lookAtVector = XMVectorAdd(positionVector, lookAtVector); //TODO : need
    // SetLookAt

    // Finally create the view matrix from the three updated vectors.
    view = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);

    return;
}

EnumBehaviorTreeStatus InitializeCamera::OnInvoke() {
    IDataBlock *block = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(block);
    assert(manager != nullptr);

    // Create and initialize the camera object.
    manager->camera = std::make_unique<Camera>();

    manager->camera->position =
        DirectX::SimpleMath::Vector3(0.0f, 0.0f, -10.0f);
    manager->camera->Render();

    // Update the position and rotation of the camera for this scene.
    manager->camera->position =
        DirectX::SimpleMath::Vector3(-10.0f, 0.0f, -10.0f);
    manager->camera->rotation = DirectX::SimpleMath::Vector3(0.0f, 45.0f, 0.0f);

    return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus UpdateCamera::OnInvoke() {
    IDataBlock *managerBlock = data_block[EnumDataBlockType::eManager];

    auto manager = dynamic_cast<Engine::PipelineManager *>(managerBlock);
    assert(manager != nullptr);

    // Generate the view matrix based on the camera's position.
    manager->camera->Render();

    return EnumBehaviorTreeStatus::eSuccess;
}
