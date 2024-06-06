#ifndef _APPLICATION
#define _APPLICATION

#include "Direct3D.h"
#include "ImGui/ImGuiManager.h"
#include "BehaviorTree.h"
#include "BehaviorLeaf.h"
#include "PipelineManager.h"
#include "Env.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

namespace Engine
{
	class Application
	{
	public:
		bool Initialize(int screenWidth,int screenHeight, HWND mainWindow);
		void Shutdown();
		bool Frame(std::unique_ptr<Input>& input);

		PipelineManager* GetManager() { return m_manager; }

	private:
		bool Render();

	private:
		ImGuiManager* m_imgui;
		PipelineManager* m_manager;
		Env* m_env;
	};
}
#endif