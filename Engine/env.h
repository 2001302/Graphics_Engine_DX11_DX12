#ifndef _ENV
#define _ENV

#include <windows.h>
#include <cmath>		//for calculate coordination

const double PI = std::acos(-1);

namespace common {
struct Env {
  private:
    Env() : main_window(0), screen_width(1920), screen_height(1080), aspect(0.0f){};
  public:
    static Env &Instance() {
        static Env instance;
        return instance;
    }

    const bool full_screen = false;
    const bool vsync_enabled = true;
    const float field_of_view = PI / 4.0f;
    const float screen_depth = 1000.0f;
    const float screen_near = 0.3f;

    HWND main_window;
    float screen_width;
    float screen_height;
    float aspect;
};
} // namespace Engine
#endif
