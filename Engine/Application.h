#pragma once

#ifndef _APPLICATION
#define _APPLICATION

#include "D3dclass.h"
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
		Application();
		Application(const Application&);
		~Application();

		bool Initialize(int, int, HWND);
		void Shutdown();
		bool Frame(std::unique_ptr<InputClass>& input);

		Manager* GetManager() { return m_Manager; }

	private:
		bool Render();

	private:
		ImGuiManager* m_Imgui;
		Manager* m_Manager;
		ViewingPoint* m_ViewingPoint;
	};
}
#endif