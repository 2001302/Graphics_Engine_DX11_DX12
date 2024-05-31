#ifndef _INPUT
#define _INPUT

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

#include "CommonStruct.h"

class Input
{
public:
	Input();
	Input(const Input&);
	~Input();

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool Frame();

	bool IsEscapePressed();
	bool IsLeftArrowPressed();
	bool IsRightArrowPressed();
	Eigen::Vector2d GetMouseLocation();
	void SetMouseLocation(int x, int y);
	bool IsMousePressed();

	IDirectInputDevice8* Mouse() {return m_mouse;}
	IDirectInputDevice8* Keyboard() { return m_keyboard; }

private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

private:
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;
	unsigned char m_keyboardState[256];
	DIMOUSESTATE m_mouseState;
	int m_screenWidth, m_screenHeight, m_mouseX, m_mouseY;
};
#endif