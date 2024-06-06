#include "Application.h"

using namespace Engine;

bool Application::Initialize(int screenWidth, int screenHeight)
{
	m_manager = new PipelineManager();
	m_env = new Env();
	m_imgui = new ImGuiManager();

	m_env->screenWidth = screenWidth;
	m_env->screenHeight = screenHeight;

	std::map<EnumDataBlockType, IDataBlock*> dataBlock =
	{
		{EnumDataBlockType::eManager,m_manager},
		{EnumDataBlockType::eGui,m_imgui},
	};

	Direct3D::GetInstance().Init(m_env, VSYNC_ENABLED, m_mainWindow, FULL_SCREEN);

	auto tree = new BehaviorTreeBuilder();

	tree->Build(dataBlock)
			->Sequence()
				->Excute(std::make_shared<InitializeCamera>())
				->Excute(std::make_shared<InitializeLight>())
				->Excute(std::make_shared<InitializeLightShader>(m_mainWindow))
			->Close();

	tree->Run();

	m_imgui->Initialize(m_mainWindow, &Direct3D::GetInstance());

	return true;
}

bool Application::Render()
{
	std::map<EnumDataBlockType, IDataBlock*> dataBlock =
	{
		{EnumDataBlockType::eManager, m_manager},
		{EnumDataBlockType::eEnv, m_env},
		{EnumDataBlockType::eGui,m_imgui},
	};

	// Clear the buffers to begin the scene.
	Direct3D::GetInstance().BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	//ImGui
	m_imgui->Prepare(m_env);

	auto tree = std::make_unique<BehaviorTreeBuilder>();

	tree->Build(dataBlock)
			->Sequence()
				->Excute(std::make_shared<RenderGameObjects>())
			->Close();

	tree->Run();

	m_imgui->Render(m_mainWindow);

	// Present the rendered scene to the screen.
	Direct3D::GetInstance().EndScene();

	return true;
}

bool Application::Frame(std::unique_ptr<Input>& input)
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
	if (m_manager)
	{
		for (auto& model : m_manager->Models)
		{
			delete model;
			model = 0;
		}

		m_manager->Light.reset();
		m_manager->LightShader.reset();
		m_manager->Camera.reset();
	}

	if (m_env)
	{
		delete m_env;
		m_env = 0;
	}

	if (m_imgui)
	{
		m_imgui->Shutdown();
		delete m_imgui;
		m_imgui = 0;
	}

	Direct3D::GetInstance().Shutdown();

	return;
}
