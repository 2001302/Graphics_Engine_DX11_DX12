#include "platform.h"

namespace foundation {

/// <summary>
/// NOTE : Global
/// </summary>
static Platform *g_system = nullptr;

Platform::Platform() : application_name(0), hinstance(0) {
    g_system = this;
}

Platform::~Platform() {}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam,
                                LPARAM lparam) {
    switch (umessage) {
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    case WM_CLOSE: {
        PostQuitMessage(0);
        return 0;
    }
    default: {
        return g_system->MessageHandler(hwnd, umessage, wparam, lparam);
    }
    }
}

bool Platform::Start() {
    InitializeWindow();
    return true;
}

bool Platform::Frame() { return true; }

void Platform::Run() {
    MSG msg;
    bool done, result;

    // Initialize the message structure.
    ZeroMemory(&msg, sizeof(MSG));

    // Loop until there is a quit message from the window or the user.
    done = false;
    while (!done) {
        // Handle the windows messages.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // If windows signals to end the application then exit out.
        if (msg.message == WM_QUIT) {
            done = true;
        } else {
            // Otherwise do the frame processing.
            result = Frame();
            if (!result) {
                done = true;
            }
        }
    }

    return;
}

bool Platform::Stop() {
    // Shutdown the window.
    if (foundation::Env::Instance().full_screen) {
        ChangeDisplaySettings(NULL, 0);
    }

    // Remove the window.
    DestroyWindow(foundation::Env::Instance().main_window);
    foundation::Env::Instance().main_window = NULL;

    // Remove the application instance.
    UnregisterClass(application_name, hinstance);
    hinstance = NULL;

    // Release the pointer to this class.
    g_system = NULL;
    return true;
}

bool Platform::InitializeWindow() {
    SetProcessDpiAwareness(PROCESS_DPI_AWARENESS::PROCESS_SYSTEM_DPI_AWARE);

    hinstance = GetModuleHandle(NULL);

    WNDCLASSEX wc = {sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     WndProc,
                     0L,
                     0L,
                     hinstance,
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     L"Engine", // lpszClassName, L-string
                     NULL};

    // The RegisterClass function has been superseded by the RegisterClassEx
    // function.
    // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassa?redirectedfrom=MSDN
    if (!RegisterClassEx(&wc)) {
        std::cout << "RegisterClassEx() failed." << std::endl;
        return false;
    }

    RECT wr = {0, 0, foundation::Env::Instance().screen_width,
               foundation::Env::Instance().screen_height};

    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

    foundation::Env::Instance().main_window =
        CreateWindow(wc.lpszClassName, L"Engine", WS_OVERLAPPEDWINDOW,
                     10,                 
                     10,                 
                     wr.right - wr.left, 
                     wr.bottom - wr.top, 
                     NULL, NULL, wc.hInstance, NULL);

    if (!foundation::Env::Instance().main_window) {
        std::cout << "CreateWindow() failed." << std::endl;
        return false;
    }

    ShowWindow(foundation::Env::Instance().main_window, SW_SHOWDEFAULT);
    UpdateWindow(foundation::Env::Instance().main_window);
}
} // namespace dx11