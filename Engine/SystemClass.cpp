#include "systemclass.h"

using namespace Engine;

/// <summary>
/// NOTE : Global
/// </summary>
static SystemClass* g_system = 0;

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

SystemClass::SystemClass()
{
	m_applicationName = 0;
	m_hinstance = 0;
	m_hwnd = 0;
}

SystemClass::SystemClass(const SystemClass& other)
{
	m_applicationName = other.m_applicationName;
	m_hinstance = other.m_hinstance;
	m_hwnd = other.m_hwnd;
}

SystemClass::~SystemClass()
{
}

bool SystemClass::Initialize()
{
	// Initialize the width and height of the screen to zero before sending the variables into the function.
	int screenWidth = 0;
	int screenHeight = 0;

	// Initialize the windows api.
	InitializeWindows(screenWidth, screenHeight);

	// Create and initialize the input object.  This object will be used to handle reading the keyboard input from the user.
	m_input = std::make_unique<InputClass>();

	if (!m_input->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight))
		return false;

	// Create and initialize the application class object.  This object will handle rendering all the graphics for this application.
	m_application = std::make_unique<Application>();

	if (!m_application->Initialize(screenWidth, screenHeight, m_hwnd))
		return false;

	return true;
}

void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX windowClass;
	DEVMODE screenSettings;
	int posX, posY;

	// Get an external pointer to this object.	
	g_system = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_applicationName = L"Engine";

	// Setup the windows class with default settings.
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = WndProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = m_hinstance;
	windowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	windowClass.hIconSm = windowClass.hIcon;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = m_applicationName;
	windowClass.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&windowClass);

	// Determine the resolution of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&screenSettings, 0, sizeof(screenSettings));
		screenSettings.dmSize = sizeof(screenSettings);
		screenSettings.dmPelsWidth = (unsigned long)screenWidth;
		screenSettings.dmPelsHeight = (unsigned long)screenHeight;
		screenSettings.dmBitsPerPel = 32;
		screenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		float aspect = 9.0f / 16.0f;
		// If windowed then set it to 800x600 resolution.
		screenWidth = 1280;//screenWidth * (3.0f / 4.0f);
		screenHeight = screenWidth * aspect;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		WS_CLIPSIBLINGS /*WS_OVERLAPPEDWINDOW*/ | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	return;
}

void SystemClass::Run()
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

bool SystemClass::Frame()
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

void SystemClass::ShutdownWindows()
{
	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	g_system = NULL;

	return;
}

void SystemClass::Shutdown()
{
	// Release the application class object.
	m_application.reset();

	// Release the input object.
	m_input.reset();

	// Shutdown the window.
	ShutdownWindows();

	return;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
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

bool SystemClass::OnModelLoadRequest()
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

			ResourceHelper::ImportModel(m_application->GetManager()->Models.back(), modelFile[i].c_str());
			ResourceHelper::ImportTexture(m_application->GetManager()->Models.back(), m_application->GetManager()->Texture);

			std::vector<VertexType> vertices;
			std::vector<int> indices;

			for (auto mesh : m_application->GetManager()->Models.back()->meshes)
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
					Direct3D::GetInstance().GetDevice()->CreateBuffer(&bufferDesc, &vertexBufferData, &m_application->GetManager()->Models.back()->vertexBuffer);
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

				Direct3D::GetInstance().GetDevice()->CreateBuffer(&bufferDesc, &indexBufferData, &m_application->GetManager()->Models.back()->indexBuffer);
			}

			//Direct3D::GetInstance().CreateVertexBuffer(vertices, m_application->GetManager()->Models.back()->vertexBuffer);
			//Direct3D::GetInstance().CreateIndexBuffer(indices, m_application->GetManager()->Models.back()->indexBuffer);

			m_application->GetManager()->Models.back()->transform = matrix[i];
		}
	}
	else
	{
		//need logger
	}
	return true;
}

bool SystemClass::OnRightDragRequest()
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

		Eigen::Vector3d origin(m_application->GetManager()->Camera->GetPosition().x, m_application->GetManager()->Camera->GetPosition().y, m_application->GetManager()->Camera->GetPosition().z);

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

		m_application->GetManager()->Camera->SetPosition(origin_prime.x(), origin_prime.y(), origin_prime.z());
	}
	
	return true;
}

bool SystemClass::OnMouseWheelRequest()
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
		Eigen::Vector3d origin(m_application->GetManager()->Camera->GetPosition().x, m_application->GetManager()->Camera->GetPosition().y, m_application->GetManager()->Camera->GetPosition().z);

		Eigen::Matrix3d R1;
		R1 <<	1.0 + wheel, 0.0, 0.0,
				0.0, 1.0 + wheel, 0.0,
				0.0, 0.0, 1.0 + wheel;

		Eigen::Vector3d origin_prime = R1* origin;
		m_application->GetManager()->Camera->SetPosition(origin_prime.x(), origin_prime.y(), origin_prime.z());
	}

	return true;
}