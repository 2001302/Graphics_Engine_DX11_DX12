#pragma once

#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_

#include "d3dclass.h"
#include "inputclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "lightclass.h"
#include "rendertextureclass.h"
#include "lightshaderclass.h"
#include "refractionshaderclass.h"
#include "watershaderclass.h"
#include "ImGuiManager.h"
#include "BehaviorTree.h"
#include "BehaviorLeaf.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

namespace Engine
{
	class Manager : public IDataBlock
	{
	public:
		std::vector <ModelClass*> Models;
		LightClass* Light;
		LightShaderClass* LightShader;
		CameraClass* Camera;
	};

	class ApplicationClass
	{
	public:
		ApplicationClass();
		ApplicationClass(const ApplicationClass&);
		~ApplicationClass();

		bool Initialize(int, int, HWND);
		void Shutdown();
		bool Frame(InputClass*);

	private:
		bool Render();

	private:
		ImGuiManager* m_Imgui;
		Manager* m_Manager;
	};

#endif
}