#ifndef _ENV
#define _ENV

#include <cmath> //for calculate coordination
#include <windows.h>

const double PI = std::acos(-1);

namespace common {
struct Env {
  private:
    Env() : main_window(0), screen_width(1920), screen_height(1080){};

  public:
    static Env &Instance() {
        static Env instance;
        return instance;
    }
    float GetAspect() { return ((float)screen_width / (float)screen_height); }

    const bool full_screen = false;
    const bool vsync_enabled = true;
    const float field_of_view = PI / 4.0f;
    const float screen_depth = 1000.0f;
    const float screen_near = 0.3f;

    HWND main_window;
    float screen_width;
    float screen_height;
};
} // namespace common
#endif
