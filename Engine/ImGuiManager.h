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

	bool Initialize(HWND m_hWnd, Engine::D3DClass* d3d);
	bool Prepare();
	bool Render();
	void Shutdown();
	void ShowWindow(bool* p_open = NULL);

private:
	HWND m_window;
};
