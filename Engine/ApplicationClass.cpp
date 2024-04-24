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
	m_Manager = new Manager();

	std::map<EnumDataBlockType, IDataBlock*> dataBlock = 
	{
		{EnumDataBlockType::eManager,m_Manager},
	};

	D3DClass::GetInstance().Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);

	auto builder = new BehaviorTreeBuilder();

	builder->Build(dataBlock)
		->Sequence()
			->Excute(new LoadModelData())
			->Excute(new InitializeCamera())
			->Excute(new InitializeLight(hwnd))
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
		{EnumDataBlockType::eViewingPoint, new ViewingPoint()},
	};

	// Clear the buffers to begin the scene.
	D3DClass::GetInstance().BeginScene(EnumViewType::eScene, 0.0f, 0.0f, 0.0f, 1.0f);

	auto builder = new BehaviorTreeBuilder();

	builder->Build(dataBlock)
		->Sequence()
			->Excute(new GetViewingPoint())
			->Excute(new RenderModels())
		->Close();

	builder->Run();

	//ImGui
	m_Imgui->Prepare();
	m_Imgui->Render();

	// Present the rendered scene to the screen.
	D3DClass::GetInstance().EndScene();

	return true;
}

bool ApplicationClass::Frame(InputClass* input)
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

	if(m_Imgui)
	{
		m_Imgui->Shutdown();
	}

	D3DClass::GetInstance().Shutdown();

	return;
}

bool ApplicationClass::OnModelLoadRequest()
{
	OPENFILENAME ofn;
	wchar_t* szFile = new wchar_t[240];

	// OPENFILENAME struct initialize
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = WindowHandler::GetInstance().GetHandle();
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = L"All files\0*.*\0Text Files\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	//show file explorer
	if (GetOpenFileName(&ofn) == TRUE) {
		//do something
	}

	return true;
}
