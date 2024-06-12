#ifndef _PLATFORM
#define _PLATFORM

#define WIN32_LEAN_AND_MEAN

#include "application.h"

namespace Engine
{
	class Platform
	{
	public:
		Platform();
		Platform(const Platform&);
		~Platform();

		bool Initialize();
		bool InitMainWindow();
		bool Frame();
		void Run();

		LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
	private:
		int m_screenWidth;
		int m_screenHeight;
		LPCWSTR m_applicationName;
		HINSTANCE m_hinstance;
		HWND m_mainWindow;

		std::unique_ptr<Application> m_application;

	};
}
#endif