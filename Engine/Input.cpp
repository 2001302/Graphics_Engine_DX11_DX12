#include "input.h"

namespace engine {

bool Input::Initialize(HINSTANCE hinstance) {
    HRESULT result;

    // Store the screen size which will be used for positioning the mouse
    // cursor.
    screen_width = common::Env::Instance().screen_width;
    screen_height = common::Env::Instance().screen_height;

    // Initialize the location of the mouse on the screen.
    mouse_x = 0;
    mouse_y = 0;

    // Initialize the main direct input interface.
    result =
        DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
                           (void **)&direct_input_, NULL);
    if (FAILED(result)) {
        return false;
    }

    // Initialize the direct input interface for the keyboard.
    result = direct_input_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
    if (FAILED(result)) {
        return false;
    }

    // Set the data format.  In this case since it is a keyboard we can use the
    // predefined data format.
    result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(result)) {
        return false;
    }

    // Set the cooperative level of the keyboard to not share with other
    // programs.
    result = keyboard_->SetCooperativeLevel(common::Env::Instance().main_window,
                                            DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    if (FAILED(result)) {
        return false;
    }

    // Now acquire the keyboard.
    result = keyboard_->Acquire();
    if (FAILED(result)) {
        return false;
    }

    // Initialize the direct input interface for the mouse.
    result = direct_input_->CreateDevice(GUID_SysMouse, &mouse_, NULL);
    if (FAILED(result)) {
        return false;
    }

    // Set the data format for the mouse using the pre-defined mouse data
    // format.
    result = mouse_->SetDataFormat(&c_dfDIMouse);
    if (FAILED(result)) {
        return false;
    }

    // Set the cooperative level of the mouse to share with other programs.
    result = mouse_->SetCooperativeLevel(common::Env::Instance().main_window,
                                         DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(result)) {
        return false;
    }

    // Acquire the mouse.
    result = mouse_->Acquire();
    if (FAILED(result)) {
        return false;
    }

    return true;
}

bool Input::Frame() {
    bool result;

    // Read the current state of the keyboard.
    result = ReadKeyboard();
    if (!result) {
        return false;
    }
    // Read the current state of the mouse.
    result = ReadMouse();
    if (!result) {
        return false;
    }

    // Process the changes in the mouse and keyboard.
    ProcessInput();

    return true;
}

bool Input::ReadKeyboard() {
    HRESULT result;

    // Read the keyboard device.
    result = keyboard_->GetDeviceState(sizeof(keyboard_state_),
                                       (LPVOID)&keyboard_state_);
    if (FAILED(result)) {
        // If the keyboard lost focus or was not acquired then try to get
        // control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
            keyboard_->Acquire();
        } else {
            return false;
        }
    }

    return true;
}

bool Input::ReadMouse() {
    HRESULT result;

    // Read the mouse device.
    result =
        mouse_->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mouse_state_);
    if (FAILED(result)) {
        // If the mouse lost focus or was not acquired then try to get control
        // back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
            mouse_->Acquire();
        } else {
            return false;
        }
    }

    return true;
}

void Input::ProcessInput() {
    // Update the location of the mouse cursor based on the change of the mouse
    // location during the frame.
    mouse_x += mouse_state_.lX;
    mouse_y += mouse_state_.lY;

    // Ensure the mouse location doesn't exceed the screen width or height.
    if (mouse_x < 0) {
        mouse_x = 0;
    }
    if (mouse_y < 0) {
        mouse_y = 0;
    }

    if (mouse_x > screen_width) {
        mouse_x = screen_width;
    }
    if (mouse_y > screen_height) {
        mouse_y = screen_height;
    }

    return;
}

bool Input::IsEscapePressed() {
    // Do a bitwise and on the keyboard state to check if the escape key is
    // currently being pressed.
    if (keyboard_state_[DIK_ESCAPE] & 0x80) {
        return true;
    }

    return false;
}

bool Input::IsLeftArrowPressed() {
    if (keyboard_state_[DIK_LEFT] & 0x80) {
        return true;
    }

    return false;
}

bool Input::IsRightArrowPressed() {
    if (keyboard_state_[DIK_RIGHT] & 0x80) {
        return true;
    }

    return false;
}

DirectX::SimpleMath::Vector2 Input::GetMouseLocation() {
    return DirectX::SimpleMath::Vector2((float)mouse_x, (float)mouse_y);
}

void Input::SetMouseLocation(int x, int y) {
    mouse_x = x;
    mouse_y = y;
};
bool Input::IsMousePressed() {
    // Check the left mouse button state.
    if (mouse_state_.rgbButtons[0] & 0x80) {
        return true;
    }

    return false;
}

void Input::Shutdown() {
    // Release the mouse.
    if (mouse_) {
        mouse_->Unacquire();
        mouse_->Release();
        mouse_ = 0;
    }

    // Release the keyboard.
    if (keyboard_) {
        keyboard_->Unacquire();
        keyboard_->Release();
        keyboard_ = 0;
    }

    // Release the main interface to direct input.
    if (direct_input_) {
        direct_input_->Release();
        direct_input_ = 0;
    }

    return;
}
} // namespace engine