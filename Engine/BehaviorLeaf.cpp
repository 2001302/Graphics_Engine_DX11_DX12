#include "BehaviorLeaf.h"

using namespace Engine;

EnumBehaviorTreeStatus LoadTextureData::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	const char* textureFilename = "../Engine/data/wall01.tga"; //TODO : need path policy
	manager->Texture = std::make_shared<TextureClass>();
	manager->Texture->Initialize(D3DManager::GetInstance().GetDevice(), D3DManager::GetInstance().GetDeviceContext(), textureFilename);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializeCamera::Invoke() 
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	// Create and initialize the camera object.
	manager->Camera =std::make_unique<CameraClass>();

	manager->Camera->SetPosition(0.0f, 0.0f, -10.0f);
	manager->Camera->Render();

	// Update the position and rotation of the camera for this scene.
	manager->Camera->SetPosition(-200.0f, 0.0f, -200.0f);
	manager->Camera->SetRotation(0.0f, 45.0f, 0.0f);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializeLight::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	// Create and initialize the light object.
	manager->Light = std::make_unique<Light>();

	manager->Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	manager->Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	manager->Light->SetDirection(0.0f, -1.0f, 0.5f);

	// Create and initialize the light shader object.
	manager->LightShader = std::make_unique<LightShader>();
	manager->LightShader->Initialize(D3DManager::GetInstance().GetDevice(), m_window);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus GetViewingPoint::Invoke()
{
	IDataBlock* managerBlock = DataBlock[EnumDataBlockType::eManager];
	IDataBlock* viewBlock = DataBlock[EnumDataBlockType::eViewingPoint];
	
	auto manager = dynamic_cast<Engine::PipelineManager*>(managerBlock);
	assert(manager != nullptr);

	auto viewingPoint = dynamic_cast<Engine::ViewingPoint*>(viewBlock);
	assert(viewingPoint != nullptr);

	// Generate the view matrix based on the camera's position.
	manager->Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	D3DManager::GetInstance().GetWorldMatrix(EnumViewType::eScene, viewingPoint->WorldMatrix);
	manager->Camera->GetViewMatrix(viewingPoint->ViewMatrix);
	D3DManager::GetInstance().GetProjectionMatrix(EnumViewType::eScene, viewingPoint->ProjectionMatrix);

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus RenderGameObjects::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];
	IDataBlock* viewBlock = DataBlock[EnumDataBlockType::eViewingPoint];

	auto manager = dynamic_cast<Engine::PipelineManager*>(block);
	assert(manager != nullptr);

	auto viewingPoint = dynamic_cast<Engine::ViewingPoint*>(viewBlock);
	assert(viewingPoint != nullptr);

	for (auto& model : manager->Models)
	{
		model->Render(D3DManager::GetInstance().GetDeviceContext());

		viewingPoint->WorldMatrix = model->transform;

		manager->LightShader->Render(D3DManager::GetInstance().GetDeviceContext(), model->GetIndexCount(), viewingPoint->WorldMatrix, viewingPoint->ViewMatrix, viewingPoint->ProjectionMatrix,
			model->texture->GetTexture(), manager->Light->GetDirection(), manager->Light->GetAmbientColor(), manager->Light->GetDiffuseColor());
	}

	return EnumBehaviorTreeStatus::eSuccess;
}