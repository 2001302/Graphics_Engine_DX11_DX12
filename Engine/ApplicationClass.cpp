#include "applicationclass.h"

ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_Imgui = 0;
	m_waterHeight = 0;
	m_waterTranslation = 0;
	m_Manager = 0;
}

ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
	m_Direct3D = other.m_Direct3D;
	m_Camera = other.m_Camera;
	m_Imgui = other.m_Imgui;
	m_waterHeight = other.m_waterHeight;
	m_waterTranslation = other.m_waterTranslation;
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

	// Create and initialize the Direct3D object.
	m_Direct3D = new D3DClass();

	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);

	// Create and initialize the camera object.
	m_Camera = new CameraClass();

	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();

	// Update the position and rotation of the camera for this scene.
	m_Camera->SetPosition(-10.0f, 6.0f, -10.0f);
	m_Camera->SetRotation(0.0f, 45.0f, 0.0f);

	m_Manager = new Manager();

	for (int i = 0; i < modelFile.size(); i++) 
	{
		m_Manager->Models.push_back(new ModelClass());
		m_Manager->Models[i]->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), modelFile[i].c_str(), textureFile[i].c_str());
		m_Manager->Models[i]->defaultTransform = matrix[i];
	}

	// Create and initialize the light object.
	m_Manager->Light = new LightClass();

	m_Manager->Light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	m_Manager->Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Manager->Light->SetDirection(0.0f, -1.0f, 0.5f);

	// Create and initialize the light shader object.
	m_Manager->LightShader = new LightShaderClass();

	result = m_Manager->LightShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	result = m_Imgui->Initialize(hwnd, m_Direct3D);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the imgui object.", L"Error", MB_OK);
		return false;
	}

	// Set the height of the water.
	m_waterHeight = 2.75f;

	// Initialize the position of the water.
	m_waterTranslation = 0.0f;

	return true;
}

bool ApplicationClass::Frame(InputClass* Input)
{
	bool result;

	// Check if the user pressed escape and wants to exit the application.
	if(Input->IsEscapePressed())
	{
		return false;
	}
	
	// Render the scene as normal to the back buffer.
	result = Render();
	if(!result)
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
	m_Direct3D->BeginScene(EnumViewType::eScene, 0.0f, 0.0f, 0.0f, 1.0f);

	result = m_Imgui->Frame();
	if (!result)
	{
		return false;
	}

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(EnumViewType::eScene, worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(EnumViewType::eScene, projectionMatrix);

	for (auto& model : m_Manager->Models) 
	{
		model->Render(m_Direct3D->GetDeviceContext());

		worldMatrix = model->defaultTransform;

		result = m_Manager->LightShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, model->GetTexture(),
			m_Manager->Light->GetDirection(), m_Manager->Light->GetAmbientColor(), m_Manager->Light->GetDiffuseColor());
	}

	result = m_Imgui->Render();

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

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
	}

	// Release the camera object.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the Direct3D object.
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	return;
}
