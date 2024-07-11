#ifndef _PLATFORM
#define _PLATFORM

#define WIN32_LEAN_AND_MEAN

#include "behavior_tree.h"
#include "direct3D.h"
#include "env.h"
#include "geometry_generator.h"
#include "setting_ui.h"
#include "pipeline_manager.h"
#include "resource_helper.h"
#include "board_map.h"

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
    LPCWSTR application_name_;
    HINSTANCE hinstance_;
};
} // namespace Engine
#endif
