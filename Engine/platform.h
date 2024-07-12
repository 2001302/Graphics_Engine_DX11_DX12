#ifndef _PLATFORM
#define _PLATFORM

#define WIN32_LEAN_AND_MEAN

#include "common_struct.h"
#include "env.h"

namespace platform {
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

  protected:
    LPCWSTR application_name_;
    HINSTANCE hinstance_;
};
} // namespace platform
#endif
