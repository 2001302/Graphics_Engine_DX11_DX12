#include "system.h"

using namespace Engine;

/// <summary>
/// NOTE : Global
/// </summary>
static System* g_system = nullptr;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		return 0;
	}

	// All other messages pass to the message handler in the system class.
	default:
	{
		return g_system->MessageHandler(hwnd, umessage, wparam, lparam);
	}
	}
}

System::System()
	: m_screenWidth(1280), m_screenHeight(960), m_mainWindow(0), m_applicationName(0), m_hinstance(0)
{
	g_system = this;
}

System::System(const System& other)
	: m_screenWidth(other.m_screenWidth), m_screenHeight(other.m_screenHeight)
{
	g_system = this;

	m_mainWindow = other.m_mainWindow;
	m_applicationName = other.m_applicationName;
	m_hinstance = other.m_hinstance;
}

System::~System()
{
}

bool System::Initialize()
{
	// Initialize the windows api.
	InitMainWindow();

	// Create and initialize the input object.  This object will be used to handle reading the keyboard input from the user.
	m_input = std::make_unique<Input>();

	if (!m_input->Initialize(m_hinstance, m_mainWindow, m_screenWidth, m_screenHeight))
		return false;

	// Create and initialize the application class object.  This object will handle rendering all the graphics for this application.
	m_application = std::make_unique<Application>();

	if (!m_application->Initialize(m_screenWidth, m_screenHeight, m_mainWindow))
		return false;

	return true;
}

bool System::InitMainWindow()
{
	m_hinstance = GetModuleHandle(NULL);

	WNDCLASSEX wc = { sizeof(WNDCLASSEX),
					 CS_CLASSDC,
					 WndProc,
					 0L,
					 0L,
					 m_hinstance,
					 NULL,
					 NULL,
					 NULL,
					 NULL,
					 L"Engine",// lpszClassName, L-string
					 NULL };

	// The RegisterClass function has been superseded by the RegisterClassEx function.
	// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassa?redirectedfrom=MSDN
	if (!RegisterClassEx(&wc)) {
		std::cout << "RegisterClassEx() failed." << std::endl;
		return false;
	}

	// 툴바까지 포함한 윈도우 전체 해상도가 아니라
	// 우리가 실제로 그리는 해상도가 width x height가 되도록
	// 윈도우를 만들 해상도를 다시 계산해서 CreateWindow()에서 사용

	// 우리가 원하는 그림이 그려질 부분의 해상도
	RECT wr = { 0, 0, m_screenWidth, m_screenHeight };

	// 필요한 윈도우 크기(해상도) 계산
	// wr의 값이 바뀜
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

	// 윈도우를 만들때 위에서 계산한 wr 사용
	m_mainWindow = CreateWindow(wc.lpszClassName, L"HongLabGraphics Example", WS_OVERLAPPEDWINDOW,
		10,                // 윈도우 좌측 상단의 x 좌표
		10,                // 윈도우 좌측 상단의 y 좌표
		wr.right - wr.left, // 윈도우 가로 방향 해상도
		wr.bottom - wr.top, // 윈도우 세로 방향 해상도
		NULL, NULL, wc.hInstance, NULL);

	if (!m_mainWindow) {
		std::cout << "CreateWindow() failed." << std::endl;
		return false;
	}

	ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
	UpdateWindow(m_mainWindow);

	return true;
}

void System::Run()
{
	MSG msg;
	bool done, result;


	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	done = false;
	while (!done)
	{
		// Handle the windows messages.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			// Otherwise do the frame processing.
			result = Frame();
			if (!result)
			{
				done = true;
			}
		}
	}

	return;
}

bool System::Frame()
{
	bool result;

	// Do the input frame processing.
	result = m_input->Frame();
	if (!result)
	{
		return false;
	}

	// Do the frame processing for the application class object.
	result = m_application->Frame(m_input);
	if (!result)
	{
		return false;
	}

	return true;
}

void System::ShutdownWindows()
{
	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_mainWindow);
	m_mainWindow = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	g_system = NULL;

	return;
}

void System::Shutdown()
{
	// Release the application class object.
	m_application.reset();

	// Release the input object.
	m_input.reset();

	// Shutdown the window.
	ShutdownWindows();

	return;
}

LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	ImGui_ImplWin32_WndProcHandler(hwnd, umsg, wparam, lparam);

	//TODO : Error Code 만들것
	switch (umsg)
	{
	case WM_MODEL_LOAD:
	{
		return OnModelLoadRequest();
		break;
	}
	case WM_MOUSEMOVE:
	{
		if (wparam & MK_RBUTTON)
		{
			return OnRightDragRequest();
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		return OnMouseWheelRequest();
		break;
	}
	case WM_RBUTTONDOWN:
	{
		//return OnRightClickRequest();
		break;
	}
	case WM_LBUTTONUP:
	{
		break;
	}
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}

bool System::OnModelLoadRequest()
{
	auto ToString = [](LPWSTR lpwstr) -> std::string
		{
			if (!lpwstr)
				return std::string();

			int len = WideCharToMultiByte(CP_UTF8, 0, lpwstr, -1, NULL, 0, NULL, NULL);
			std::string result(len, '\0');
			WideCharToMultiByte(CP_UTF8, 0, lpwstr, -1, &result[0], len, NULL, NULL);
			return result;
		};


	OPENFILENAMEW  ofn;
	wchar_t szFile[260] = { 0 };

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
	if (GetOpenFileName(&ofn))
	{
		std::vector<std::string> modelFile;
		std::vector<XMMATRIX> matrix;

		modelFile.push_back(ToString(ofn.lpstrFile));
		matrix.push_back(XMMatrixTranslation(0.0f, 0.0f, 0.0f));

		for (int i = 0; i < modelFile.size(); i++)
		{
			m_application->GetManager()->Models.push_back(new GameObject());

			auto model = m_application->GetManager()->Models.back();

			ResourceHelper::ImportModel(model, modelFile[i].c_str());
			ResourceHelper::CreateTexture(model, "C:\\Users\\user\\Source\\repos\\Engine\\Engine\\data\\crate2_diffuse.png");

			std::vector<VertexType> vertices;
			std::vector<int> indices;

			for (auto mesh : model->meshes)
			{
				vertices.insert(vertices.end(), mesh->vertices.begin(), mesh->vertices.end());
				indices.insert(indices.end(), mesh->indices.begin(), mesh->indices.end());
			}

			{
				D3D11_BUFFER_DESC bufferDesc;
				ZeroMemory(&bufferDesc, sizeof(bufferDesc));
				bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
				bufferDesc.ByteWidth = sizeof(Engine::VertexType) * vertices.size(); //UINT(sizeof(T_VERTEX) * vertices.size());
				bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
				bufferDesc.StructureByteStride = sizeof(Engine::VertexType);
				bufferDesc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 }; // MS 예제에서 초기화하는 방식
				vertexBufferData.pSysMem = vertices.data();
				vertexBufferData.SysMemPitch = 0;
				vertexBufferData.SysMemSlicePitch = 0;

				const HRESULT hr =
					Direct3D::GetInstance().GetDevice()->CreateBuffer(&bufferDesc, &vertexBufferData, &model->vertexBuffer);
				if (FAILED(hr)) {
					std::cout << "CreateBuffer() failed. " << std::hex << hr << std::endl;
				};
			}
			{
				D3D11_BUFFER_DESC bufferDesc;
				bufferDesc.Usage = D3D11_USAGE_IMMUTABLE; // 초기화 후 변경X
				bufferDesc.ByteWidth = sizeof(unsigned long) * indices.size();
				bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
				bufferDesc.StructureByteStride = sizeof(int);
				bufferDesc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
				indexBufferData.pSysMem = indices.data();
				indexBufferData.SysMemPitch = 0;
				indexBufferData.SysMemSlicePitch = 0;

				Direct3D::GetInstance().GetDevice()->CreateBuffer(&bufferDesc, &indexBufferData, &model->indexBuffer);
			}

			//create constant buffer
			model->vertexConstantBufferData.world = DirectX::SimpleMath::Matrix();
			model->vertexConstantBufferData.view = DirectX::SimpleMath::Matrix();
			model->vertexConstantBufferData.projection = DirectX::SimpleMath::Matrix();

			m_application->GetManager()->LightShader->CreateConstantBuffer(model->vertexConstantBufferData,
				model->vertexConstantBuffer);
			m_application->GetManager()->LightShader->CreateConstantBuffer(model->pixelConstantBufferData,
				model->pixelConstantBuffer);

			model->transform = matrix[i];
		}
	}
	else
	{
		//need logger
	}
	return true;
}

bool System::OnRightDragRequest()
{
	DIMOUSESTATE mouseState;
	if (FAILED(m_input->Mouse()->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState)))
	{
		//retry
		m_input->Mouse()->Acquire();
	}
	else
	{
		auto viewPort = Direct3D::GetInstance().Views[EnumViewType::eScene].Viewport;

		//mouse move vector
		Eigen::Vector2d vector = Eigen::Vector2d(-mouseState.lX, -mouseState.lY);

		Eigen::Vector3d origin(m_application->GetManager()->Camera->position.x, m_application->GetManager()->Camera->position.y, m_application->GetManager()->Camera->position.z);

		//convert to spherical coordinates
		double r = origin.norm();
		double phi = acos(origin.y() / r);
		double theta = atan2(origin.z(), origin.x());

		//rotation
		double deltaTheta = (2 * M_PI) * (vector.x() / viewPort.Width);
		double deltaPhi = (2 * M_PI) * (vector.y() / viewPort.Width);

		theta += deltaTheta;
		phi += deltaPhi;

		//convert to Cartesian coordinates after rotation
		double x = r * sin(phi) * cos(theta);
		double y = r * cos(phi);
		double z = r * sin(phi) * sin(theta);

		Eigen::Vector3d origin_prime(x, y, z);

		m_application->GetManager()->Camera->position = DirectX::SimpleMath::Vector3(origin_prime.x(), origin_prime.y(), origin_prime.z());
	}

	return true;
}

bool System::OnMouseWheelRequest()
{
	DIMOUSESTATE mouseState;
	if (FAILED(m_input->Mouse()->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState)))
	{
		//retry
		m_input->Mouse()->Acquire();
	}
	else
	{
		double wheel = -mouseState.lZ/(600.0);
		Eigen::Vector3d origin(m_application->GetManager()->Camera->position.x, m_application->GetManager()->Camera->position.y, m_application->GetManager()->Camera->position.z);

		Eigen::Matrix3d R1;
		R1 <<	1.0 + wheel, 0.0, 0.0,
				0.0, 1.0 + wheel, 0.0,
				0.0, 0.0, 1.0 + wheel;

		Eigen::Vector3d origin_prime = R1* origin;
		m_application->GetManager()->Camera->position = DirectX::SimpleMath::Vector3(origin_prime.x(), origin_prime.y(), origin_prime.z());
	}

	return true;
}