#ifndef _INPUT
#define _INPUT

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "graphics_manager.h"
#include "env.h"
#include "dataBlock.h"
#include <dinput.h>

namespace engine {
class Input : public common::IDataBlock {
  public:
    Input();
    Input(const Input &);

    bool Initialize(HINSTANCE);
    void Shutdown();
    bool Frame();

    bool IsEscapePressed();
    bool IsLeftArrowPressed();
    bool IsRightArrowPressed();
    DirectX::SimpleMath::Vector2 GetMouseLocation();
    void SetMouseLocation(int x, int y);
    bool IsMousePressed();

    IDirectInputDevice8 *Mouse() { return mouse_; }
    IDirectInputDevice8 *Keyboard() { return keyboard_; }

  private:
    bool ReadKeyboard();
    bool ReadMouse();
    void ProcessInput();

  private:
    IDirectInput8 *direct_input_;
    IDirectInputDevice8 *keyboard_;
    IDirectInputDevice8 *mouse_;
    unsigned char keyboard_state_[256];
    DIMOUSESTATE mouse_state_;
    int screen_width, screen_height, mouse_x, mouse_y;
};
} // namespace dx11
#endif
