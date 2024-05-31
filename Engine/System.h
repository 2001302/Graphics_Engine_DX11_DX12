#ifndef _SYSTEM
#define _SYSTEM

#define WIN32_LEAN_AND_MEAN

#include "input.h"
#include "application.h"
#include "resourcehelper.h"

namespace Engine
{
	class System
	{
	public:
		System();
		System(const System&);
		~System();

		bool Initialize();
		void Shutdown();
		void Run();

		bool OnModelLoadRequest();
		bool OnRightDragRequest();
		bool OnMouseWheelRequest();

		LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	private:
		bool Frame();
		bool InitMainWindow();
		void ShutdownWindows();

	private:
		int m_screenWidth;
		int m_screenHeight;
		LPCWSTR m_applicationName;
		HINSTANCE m_hinstance;
		HWND m_mainWindow;

		std::unique_ptr<Input> m_input;
		std::unique_ptr<Application> m_application;
	};
}
#endif