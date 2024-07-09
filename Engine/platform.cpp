#include "platform.h"

using namespace Engine;

/// <summary>
/// NOTE : Global
/// </summary>
static Platform *g_system = nullptr;

Platform::Platform() : main_window_(0), application_name_(0), hinstance_(0) {
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

bool Platform::OnStart() {

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

    // 툴바까지 포함한 윈도우 전체 해상도가 아니라
    // 우리가 실제로 그리는 해상도가 width x height가 되도록
    // 윈도우를 만들 해상도를 다시 계산해서 CreateWindow()에서 사용

    // 우리가 원하는 그림이 그려질 부분의 해상도
    RECT wr = {0, 0, Env::Get().screen_width, Env::Get().screen_height};

    // 필요한 윈도우 크기(해상도) 계산
    // wr의 값이 바뀜
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

    // 윈도우를 만들때 위에서 계산한 wr 사용
    main_window_ =
        CreateWindow(wc.lpszClassName, L"Engine", WS_OVERLAPPEDWINDOW,
                     10,                 // 윈도우 좌측 상단의 x 좌표
                     10,                 // 윈도우 좌측 상단의 y 좌표
                     wr.right - wr.left, // 윈도우 가로 방향 해상도
                     wr.bottom - wr.top, // 윈도우 세로 방향 해상도
                     NULL, NULL, wc.hInstance, NULL);

    if (!main_window_) {
        std::cout << "CreateWindow() failed." << std::endl;
        return false;
    }

    ShowWindow(main_window_, SW_SHOWDEFAULT);
    UpdateWindow(main_window_);

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
    if (FULL_SCREEN) {
        ChangeDisplaySettings(NULL, 0);
    }

    // Remove the window.
    DestroyWindow(main_window_);
    main_window_ = NULL;

    // Remove the application instance.
    UnregisterClass(application_name_, hinstance_);
    hinstance_ = NULL;

    // Release the pointer to this class.
    g_system = NULL;
    return true;
}
