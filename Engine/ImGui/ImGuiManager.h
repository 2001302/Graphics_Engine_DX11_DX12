#include "../Direct3D.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

class ImGuiManager
{
public:
	ImGuiManager() {};
	ImGuiManager(const ImGuiManager&) {};
	~ImGuiManager() {};

	bool Initialize(HWND m_hWnd, Engine::Direct3D* d3d);
	bool Prepare();
	bool Render();
	void Shutdown();

private:
	void SetupImGuiStyle(bool styleDark, float alpha);
};

class WindowHandler
{
private:
	WindowHandler() { x = 0; }
	WindowHandler(const WindowHandler& ref) { x = ref.x; }
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