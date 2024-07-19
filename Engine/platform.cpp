#include "platform.h"

namespace dx11 {

/// <summary>
/// NOTE : Global
/// </summary>
static Platform *g_system = nullptr;

Platform::Platform() : application_name_(0), hinstance_(0) { g_system = this; }

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

bool Platform::OnStart() {
    InitializeWindow();
    InitializeDirectX();
    return true;
}

bool Platform::OnFrame() { return true; }

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
            result = OnFrame();
            if (!result) {
                done = true;
            }
        }
    }

    return;
}

bool Platform::OnStop() {
    // Shutdown the window.
    if (common::Env::Instance().full_screen) {
        ChangeDisplaySettings(NULL, 0);
    }

    // Remove the window.
    DestroyWindow(common::Env::Instance().main_window);
    common::Env::Instance().main_window = NULL;

    // Remove the application instance.
    UnregisterClass(application_name_, hinstance_);
    hinstance_ = NULL;

    // Release the pointer to this class.
    g_system = NULL;
    return true;
}

bool Platform::InitializeWindow() {
    SetProcessDpiAwareness(PROCESS_DPI_AWARENESS::PROCESS_SYSTEM_DPI_AWARE);

    hinstance_ = GetModuleHandle(NULL);

    WNDCLASSEX wc = {sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     WndProc,
                     0L,
                     0L,
                     hinstance_,
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

    RECT wr = {0, 0, common::Env::Instance().screen_width,
               common::Env::Instance().screen_height};

    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

    common::Env::Instance().main_window =
        CreateWindow(wc.lpszClassName, L"Engine", WS_OVERLAPPEDWINDOW,
                     10,                 // 윈도우 좌측 상단의 x 좌표
                     10,                 // 윈도우 좌측 상단의 y 좌표
                     wr.right - wr.left, // 윈도우 가로 방향 해상도
                     wr.bottom - wr.top, // 윈도우 세로 방향 해상도
                     NULL, NULL, wc.hInstance, NULL);

    if (!common::Env::Instance().main_window) {
        std::cout << "CreateWindow() failed." << std::endl;
        return false;
    }

    ShowWindow(common::Env::Instance().main_window, SW_SHOWDEFAULT);
    UpdateWindow(common::Env::Instance().main_window);
}

bool Platform::InitializeDirectX() { return true; }
} // namespace platform