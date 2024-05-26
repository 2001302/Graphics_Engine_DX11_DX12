#ifndef _APPLICATION
#define _APPLICATION

#include "Direct3D.h"
#include "CommonModel.h"
#include "ImGui/ImGuiManager.h"
#include "BehaviorTree.h"
#include "BehaviorLeaf.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

namespace Engine
{
	class Application
	{
	public:
		bool Initialize(int, int, HWND);
		void Shutdown();
		bool Frame(std::unique_ptr<InputClass>& input);

		PipelineManager* GetManager() { return m_manager; }

	private:
		bool Render();

	private:
		ImGuiManager* m_imgui;
		PipelineManager* m_manager;
		ViewingPoint* m_viewingPoint;
	};
}
#endif