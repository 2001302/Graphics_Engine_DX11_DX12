#include "Applicationclass.h"

using namespace Engine;

ApplicationClass::ApplicationClass()
{
	m_Imgui = 0;
	m_Manager = 0;
	m_ViewingPoint = 0;
}

ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
	m_Imgui = other.m_Imgui;
	m_Manager = other.m_Manager;
	m_ViewingPoint = other.m_ViewingPoint;
}

ApplicationClass::~ApplicationClass()
{
}

bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	m_Manager = new Manager();
	m_ViewingPoint = new ViewingPoint();

	std::map<EnumDataBlockType, IDataBlock*> dataBlock = 
	{
		{EnumDataBlockType::eManager,m_Manager},
	};

	D3DClass::GetInstance().Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);

	auto builder = new BehaviorTreeBuilder();

	builder ->Build(dataBlock)
				->Sequence()
					->Excute(std::make_shared<LoadTextureData>())
					->Excute(std::make_shared<InitializeCamera>())
					->Excute(std::make_shared<InitializeLight>(hwnd))
				->Close();

	builder->Run();

	m_Imgui->Initialize(hwnd, &D3DClass::GetInstance());

	return true;
}

bool ApplicationClass::Render()
{
	std::map<EnumDataBlockType, IDataBlock*> dataBlock =
	{
		{EnumDataBlockType::eManager, m_Manager},
		{EnumDataBlockType::eViewingPoint, m_ViewingPoint},
	};

	// Clear the buffers to begin the scene.
	D3DClass::GetInstance().BeginScene(EnumViewType::eScene, 0.0f, 0.0f, 0.0f, 1.0f);

	auto builder = std::make_unique<BehaviorTreeBuilder>();

	builder->Build(dataBlock)
				->Sequence()
					->Excute(std::make_shared<GetViewingPoint>())
					->Excute(std::make_shared<RenderModels>())
				->Close();

	builder->Run();

	//ImGui
	m_Imgui->Prepare();
	m_Imgui->Render();

	// Present the rendered scene to the screen.
	D3DClass::GetInstance().EndScene();

	return true;
}

bool ApplicationClass::Frame(std::unique_ptr<InputClass>& input)
{
	bool result;

	// Check if the user pressed escape and wants to exit the application.
	if (input->IsEscapePressed())
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

		m_Manager->Light.reset();

		m_Manager->LightShader->Shutdown();
		m_Manager->LightShader.reset();

		m_Manager->Camera.reset();
	}

	if (m_ViewingPoint)
	{
		delete m_ViewingPoint;
		m_ViewingPoint = 0;
	}

	if(m_Imgui)
	{
		m_Imgui->Shutdown();
	}

	D3DClass::GetInstance().Shutdown();

	return;
}
