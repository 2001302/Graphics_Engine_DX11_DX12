﻿#ifndef _ENV
#define _ENV

#include <cmath>
#include <windows.h>

const double PI = std::acos(-1);

namespace foundation {
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
    const double field_of_view = PI / 4.0f;
    const float screen_depth = 1000.0f;
    const float screen_near = 0.3f;

    HWND main_window;
    int screen_width;
    int screen_height;
};
} // namespace common
#endif