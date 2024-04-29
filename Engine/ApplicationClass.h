#pragma once

#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_

#include "D3dclass.h"
#include "CommonModel.h"
#include "ImGuiManager.h"
#include "BehaviorTree.h"
#include "BehaviorLeaf.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

namespace Engine
{

	class ApplicationClass
	{
	public:
		ApplicationClass();
		ApplicationClass(const ApplicationClass&);
		~ApplicationClass();

		bool Initialize(int, int, HWND);
		void Shutdown();
		bool Frame(std::unique_ptr<InputClass>& input);

		Manager* GetManager() { return m_Manager; }

		bool OnModelLoadRequest(); 

	private:
		bool Render();

	private:
		ImGuiManager* m_Imgui;
		Manager* m_Manager;
		ViewingPoint* m_ViewingPoint;
	};

#endif
}