#ifndef _PLATFORM
#define _PLATFORM

#define WIN32_LEAN_AND_MEAN

#include "behavior_tree.h"
#include "direct3D.h"
#include "env.h"
#include "geometry_generator.h"
#include "panel.h"
#include "pipeline_manager.h"
#include "resource_helper.h"
#include "post_process.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

namespace Engine {
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
    int screen_width_;
    int screen_height_;
    LPCWSTR application_name_;
    HINSTANCE hinstance_;
    HWND main_window_;
};
} // namespace Engine
#endif