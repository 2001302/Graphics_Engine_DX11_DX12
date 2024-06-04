#include "Application.h"

using namespace Engine;

bool Application::Initialize(int screenWidth, int screenHeight, HWND mainWindow)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_manager = new PipelineManager();
	m_viewingPoint = new ViewingPoint();

	std::map<EnumDataBlockType, IDataBlock*> dataBlock =
	{
		{EnumDataBlockType::eManager,m_manager},
	};

	Direct3D::GetInstance().Initialize(m_screenWidth, m_screenHeight, VSYNC_ENABLED, mainWindow, FULL_SCREEN);

	auto tree = new BehaviorTreeBuilder();

	tree->Build(dataBlock)
			->Sequence()
				->Excute(std::make_shared<InitializeCamera>())
				->Excute(std::make_shared<InitializeLight>())
				->Excute(std::make_shared<InitializeShader>(mainWindow))
			->Close();

	tree->Run();

	m_imgui->Initialize(mainWindow, &Direct3D::GetInstance());

	return true;
}

bool Application::Render()
{
	std::map<EnumDataBlockType, IDataBlock*> dataBlock =
	{
		{EnumDataBlockType::eManager, m_manager},
		{EnumDataBlockType::eViewingPoint, m_viewingPoint},
	};

	// Clear the buffers to begin the scene.
	Direct3D::GetInstance().BeginScene(EnumViewType::eScene, 0.0f, 0.0f, 0.0f, 1.0f);

	//ImGui
	m_imgui->Prepare(m_screenWidth, m_screenHeight);

	auto tree = std::make_unique<BehaviorTreeBuilder>();

	tree->Build(dataBlock)
			->Sequence()
				->Excute(std::make_shared<GetViewingPoint>())
				->Excute(std::make_shared<RenderGameObjects>())
			->Close();

	tree->Run();

	m_imgui->Render();

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

		m_manager->LightShader->Shutdown();
		m_manager->LightShader.reset();

		m_manager->Camera.reset();
	}

	if (m_viewingPoint)
	{
		delete m_viewingPoint;
		m_viewingPoint = 0;
	}

	if (m_imgui)
	{
		m_imgui->Shutdown();
	}

	Direct3D::GetInstance().Shutdown();

	return;
}
