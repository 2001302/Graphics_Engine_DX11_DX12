#include "Application.h"

using namespace Engine;

Application::Application()
{
	m_Imgui = 0;
	m_Manager = 0;
	m_ViewingPoint = 0;
}

Application::Application(const Application& other)
{
	m_Imgui = other.m_Imgui;
	m_Manager = other.m_Manager;
	m_ViewingPoint = other.m_ViewingPoint;
}

Application::~Application()
{
}

bool Application::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	m_Manager = new Manager();
	m_ViewingPoint = new ViewingPoint();

	std::map<EnumDataBlockType, IDataBlock*> dataBlock =
	{
		{EnumDataBlockType::eManager,m_Manager},
	};

	D3DClass::GetInstance().Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);

	auto tree = new BehaviorTreeBuilder();

	tree->Build(dataBlock)
		->Sequence()
		->Excute(std::make_shared<LoadTextureData>())
		->Excute(std::make_shared<InitializeCamera>())
		->Excute(std::make_shared<InitializeLight>(hwnd))
		->Close();

	tree->Run();

	m_Imgui->Initialize(hwnd, &D3DClass::GetInstance());

	return true;
}

bool Application::Render()
{
	std::map<EnumDataBlockType, IDataBlock*> dataBlock =
	{
		{EnumDataBlockType::eManager, m_Manager},
		{EnumDataBlockType::eViewingPoint, m_ViewingPoint},
	};

	// Clear the buffers to begin the scene.
	D3DClass::GetInstance().BeginScene(EnumViewType::eScene, 0.2f, 0.2f, 0.2f, 1.0f);

	//ImGui
	m_Imgui->Prepare();

	//ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	//ImGui::Begin("Scene");
	//ImGui::End();

	auto tree = std::make_unique<BehaviorTreeBuilder>();

	tree->Build(dataBlock)
		->Sequence()
		->Excute(std::make_shared<GetViewingPoint>())
		->Excute(std::make_shared<RenderGameObjects>())
		->Close();

	tree->Run();

	m_Imgui->Render();

	// Present the rendered scene to the screen.
	D3DClass::GetInstance().EndScene();

	return true;
}

bool Application::Frame(std::unique_ptr<InputClass>& input)
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

void Application::Shutdown()
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
