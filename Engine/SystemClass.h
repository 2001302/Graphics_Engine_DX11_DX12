#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

#define WIN32_LEAN_AND_MEAN

#include "inputclass.h"
#include "applicationclass.h"
#include "header.h"

namespace Engine
{
	class SystemClass
	{
	public:
		SystemClass();
		SystemClass(const SystemClass&);
		~SystemClass();

		bool Initialize();
		void Shutdown();
		void Run();

		LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	private:
		bool Frame();
		void InitializeWindows(int&, int&);
		void ShutdownWindows();

	private:
		LPCWSTR m_applicationName;
		HINSTANCE m_hinstance;
		HWND m_hwnd;

		InputClass* m_Input;
		ApplicationClass* m_Application;
	};

	static SystemClass* ApplicationHandle = 0;
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
	{
		switch (umessage)
		{
			// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		// All other messages pass to the message handler in the system class.
		default:
		{
			return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
		}
	}
}
#endif