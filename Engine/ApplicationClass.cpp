#include "Applicationclass.h"

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
	bool result;

	result = D3DClass::GetInstance().Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);

	m_Manager = new Manager();

	std::map<EnumDataBlockType, IDataBlock*> dataBlock = 
	{
		{EnumDataBlockType::eManager,m_Manager},
	};

	auto builder = new BehaviorTreeBuilder();

	builder->Build(dataBlock)
		->Sequence()
			->Excute(new LoadModelData())
			->Excute(new InitializeCamera())
			->Excute(new InitializeLight(hwnd))
		->Close();

	builder->Run();

	result = m_Imgui->Initialize(hwnd, &D3DClass::GetInstance());

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
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	// Clear the buffers to begin the scene.
	D3DClass::GetInstance().BeginScene(EnumViewType::eScene, 0.0f, 0.0f, 0.0f, 1.0f);

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

	result = m_Imgui->Frame();
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
