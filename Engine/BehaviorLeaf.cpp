#include "BehaviorLeaf.h"

using namespace Engine;

EnumBehaviorTreeStatus LoadModelData::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::Manager*>(block);
	assert(manager != nullptr);

	std::vector<std::string> modelFile;
	std::vector<std::string> textureFile;
	std::vector<XMMATRIX> matrix;

	modelFile.push_back("../Engine/data/ground.txt");
	textureFile.push_back("../Engine/data/ground01.tga");
	matrix.push_back(XMMatrixTranslation(0.0f, 1.0f, 0.0f));

	modelFile.push_back("../Engine/data/wall.txt");
	textureFile.push_back("../Engine/data/wall01.tga");
	matrix.push_back(XMMatrixTranslation(0.0f, 6.0f, 8.0f));

	modelFile.push_back("../Engine/data/bath.txt");
	textureFile.push_back("../Engine/data/marble01.tga");
	matrix.push_back(XMMatrixTranslation(0.0f, 2.0f, 0.0f));

	for (int i = 0; i < modelFile.size(); i++)
	{
		manager->Models.push_back(new ModelClass());
		manager->Models[i]->Initialize(D3DClass::GetInstance().GetDevice(), D3DClass::GetInstance().GetDeviceContext(), modelFile[i].c_str(), textureFile[i].c_str());
		manager->Models[i]->defaultTransform = matrix[i];
	}

	return EnumBehaviorTreeStatus::eSuccess;
}

EnumBehaviorTreeStatus InitializeCamera::Invoke() 
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::Manager*>(block);
	assert(manager != nullptr);

	// Create and initialize the camera object.
	manager->Camera = new CameraClass();

	manager->Camera->SetPosition(0.0f, 0.0f, -10.0f);
	manager->Camera->Render();

	// Update the position and rotation of the camera for this scene.
	manager->Camera->SetPosition(-10.0f, 6.0f, -10.0f);
	manager->Camera->SetRotation(0.0f, 45.0f, 0.0f);

	return EnumBehaviorTreeStatus::eSuccess;
}


EnumBehaviorTreeStatus InitializeLight::Invoke()
{
	IDataBlock* block = DataBlock[EnumDataBlockType::eManager];

	auto manager = dynamic_cast<Engine::Manager*>(block);
	assert(manager != nullptr);

	// Create and initialize the light object.
	manager->Light = new LightClass();

	manager->Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	manager->Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	manager->Light->SetDirection(0.0f, -1.0f, 0.5f);

	// Create and initialize the light shader object.
	manager->LightShader = new LightShaderClass();

	return EnumBehaviorTreeStatus::eSuccess;
}