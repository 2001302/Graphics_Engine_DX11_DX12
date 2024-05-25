#include "Application.h"

using namespace Engine;

bool Application::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	m_Manager = new PipelineManager();
	m_ViewingPoint = new ViewingPoint();

	std::map<EnumDataBlockType, IDataBlock*> dataBlock =
	{
		{EnumDataBlockType::eManager,m_Manager},
	};

	Direct3D::GetInstance().Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);

	auto tree = new BehaviorTreeBuilder();

	tree->Build(dataBlock)
		->Sequence()
		->Excute(std::make_shared<LoadTextureData>())//TODO:제거필요
		->Excute(std::make_shared<InitializeCamera>())
		->Excute(std::make_shared<InitializeLight>(hwnd))
		//->Excute(std::make_shared<InitializeShader>())
		->Close();

	tree->Run();

	m_Imgui->Initialize(hwnd, &Direct3D::GetInstance());

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
	Direct3D::GetInstance().BeginScene(EnumViewType::eScene, 0.2f, 0.2f, 0.2f, 1.0f);

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
	Direct3D::GetInstance().EndScene();

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

	if (m_Imgui)
	{
		m_Imgui->Shutdown();
	}

	Direct3D::GetInstance().Shutdown();

	return;
}
