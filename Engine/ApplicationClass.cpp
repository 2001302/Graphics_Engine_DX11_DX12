#include "applicationclass.h"

using namespace Engine;

ApplicationClass::ApplicationClass()
{
	m_Imgui = 0;
	m_Manager = 0;
}

ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
	m_Imgui = other.m_Imgui;
	m_Manager = other.m_Manager;
}

ApplicationClass::~ApplicationClass()
{
}

bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
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

	bool result;

	result = D3DClass::GetInstance().Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);

	m_Manager = new Manager();

	// Create and initialize the camera object.
	m_Manager->Camera = new CameraClass();

	m_Manager->Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Manager->Camera->Render();

	// Update the position and rotation of the camera for this scene.
	m_Manager->Camera->SetPosition(-10.0f, 6.0f, -10.0f);
	m_Manager->Camera->SetRotation(0.0f, 45.0f, 0.0f);


	for (int i = 0; i < modelFile.size(); i++)
	{
		m_Manager->Models.push_back(new ModelClass());
		m_Manager->Models[i]->Initialize(D3DClass::GetInstance().GetDevice(), D3DClass::GetInstance().GetDeviceContext(), modelFile[i].c_str(), textureFile[i].c_str());
		m_Manager->Models[i]->defaultTransform = matrix[i];
	}

	// Create and initialize the light object.
	m_Manager->Light = new LightClass();

	m_Manager->Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	m_Manager->Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Manager->Light->SetDirection(0.0f, -1.0f, 0.5f);

	// Create and initialize the light shader object.
	m_Manager->LightShader = new LightShaderClass();

	result = m_Manager->LightShader->Initialize(D3DClass::GetInstance().GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	result = m_Imgui->Initialize(hwnd, &D3DClass::GetInstance());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the imgui object.", L"Error", MB_OK);
		return false;
	}

	return true;
}

bool ApplicationClass::Frame(InputClass* Input)
{
	bool result;

	// Check if the user pressed escape and wants to exit the application.
	if (Input->IsEscapePressed())
	{
		return false;
	}

	// Render the scene as normal to the back buffer.
	result = Render();
	if (!result)
	{
		return false;
	}
	return true;
}

bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, reflectionMatrix;
	bool result;

	// Clear the buffers to begin the scene.
	D3DClass::GetInstance().BeginScene(EnumViewType::eScene, 0.0f, 0.0f, 0.0f, 1.0f);

	result = m_Imgui->Frame();

	// Generate the view matrix based on the camera's position.
	m_Manager->Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	D3DClass::GetInstance().GetWorldMatrix(EnumViewType::eScene, worldMatrix);
	m_Manager->Camera->GetViewMatrix(viewMatrix);
	D3DClass::GetInstance().GetProjectionMatrix(EnumViewType::eScene, projectionMatrix);

	for (auto& model : m_Manager->Models)
	{
		model->Render(D3DClass::GetInstance().GetDeviceContext());

		worldMatrix = model->defaultTransform;

		result = m_Manager->LightShader->Render(D3DClass::GetInstance().GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, model->GetTexture(),
			m_Manager->Light->GetDirection(), m_Manager->Light->GetAmbientColor(), m_Manager->Light->GetDiffuseColor());
	}

	result = m_Imgui->Render();

	// Present the rendered scene to the screen.
	D3DClass::GetInstance().EndScene();

	return true;
}

void ApplicationClass::Shutdown()
{
	if (m_Manager)
	{
		for (auto& model : m_Manager->Models)
		{
			model->Shutdown();
			delete model;
			model = 0;
		}

		delete m_Manager->Light;
		m_Manager->Light = 0;

		m_Manager->LightShader->Shutdown();
		delete m_Manager->LightShader;
		m_Manager->LightShader = 0;

		if (m_Manager->Camera)
		{
			delete m_Manager->Camera;
			m_Manager->Camera = 0;
		}
	}

	D3DClass::GetInstance().Shutdown();

	return;
}
