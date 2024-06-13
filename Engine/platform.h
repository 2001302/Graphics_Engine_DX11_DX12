#ifndef _PLATFORM
#define _PLATFORM

#define WIN32_LEAN_AND_MEAN

#include "env.h"
#include "direct3D.h"
#include "imgui_manager.h"
#include "behavior_tree.h"
#include "behavior_leaf.h"
#include "pipeline_manager.h"
#include "resource_helper.h"
#include "geometry_generator.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

namespace Engine
{
	class Platform
	{
	public:
		Platform();
		~Platform();

		virtual bool OnStart();
		virtual bool OnFrame();
		virtual bool OnStop();
		void Run();
		virtual LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM) { return 0; };
	
	protected:
		int m_screenWidth;
		int m_screenHeight;
		LPCWSTR m_applicationName;
		HINSTANCE m_hinstance;
		HWND m_mainWindow;
	};
}
#endif