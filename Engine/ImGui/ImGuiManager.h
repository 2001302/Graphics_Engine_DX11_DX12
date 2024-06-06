#include "../Direct3D.h"
#include "../IDataBlock.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

namespace Engine
{
	class ImGuiManager
	{
	public:
		ImGuiManager() {};
		ImGuiManager(const ImGuiManager&) {};
		~ImGuiManager() {};

		bool Initialize(HWND mainWindow, Engine::Direct3D* d3d);
		bool Prepare(Env* aspect);
		bool Render(HWND mainWindow);
		void Shutdown();

	private:
		void SetupImGuiStyle(bool styleDark, float alpha);
	};
}