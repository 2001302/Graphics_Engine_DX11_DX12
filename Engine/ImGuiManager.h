#include "d3dclass.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

class ImGuiManager
{
public:
	ImGuiManager() {};
	ImGuiManager(const ImGuiManager&) {};
	~ImGuiManager() {};

	bool Initialize(HWND m_hWnd, D3DClass* d3d);
	void Shutdown();
	bool Frame();
	bool Render();
};
