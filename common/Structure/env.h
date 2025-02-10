#ifndef _ENV
#define _ENV

#include <cmath>
#include <windows.h>
#include <string>

const double PI = std::acos(-1);

namespace common {
namespace env {
extern std::string env_root;

extern const bool full_screen;
extern const bool vsync_enabled;
extern const double field_of_view;
extern const float screen_depth;
extern const float screen_near;

extern HWND main_window;
extern int screen_width;
extern int screen_height;

} // namespace env
} // namespace common
#endif
