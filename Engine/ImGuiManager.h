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

	void ShowWindow(HWND hwnd,bool* p_open = NULL);
};

class WindowHandler
{
private:
	WindowHandler() {}
	WindowHandler(const WindowHandler& ref) {}
	WindowHandler& operator=(const WindowHandler& ref) {}
	~WindowHandler() {}

	HWND x;

public:
	static WindowHandler& GetInstance()
	{
		static WindowHandler instance;
		return instance;
	}
	HWND GetHandle() { return x; }
	void SetHandle(HWND hwnd) 
	{	
		if(x==nullptr)
			x = hwnd; 
	}
};