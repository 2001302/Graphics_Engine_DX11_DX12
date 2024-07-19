#ifndef _PLATFORM
#define _PLATFORM

#define WIN32_LEAN_AND_MEAN

#include "env.h"
#include <iostream>
#include <shellscalingapi.h>
#pragma comment(lib, "shcore.lib")

namespace dx11 {
class Platform {
  public:
    Platform();
    ~Platform();

    virtual bool OnStart();
    virtual bool OnFrame();
    virtual bool OnStop();
    void Run();
    virtual LRESULT CALLBACK MessageHandler(HWND main_window, UINT umsg,
                                            WPARAM wparam, LPARAM lparam) {
        return 0;
    };

  private:
    bool InitializeWindow();
    bool InitializeDirectX();

  protected:
    LPCWSTR application_name_;
    HINSTANCE hinstance_;
};
} // namespace platform
#endif
