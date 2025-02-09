#include "env.h"

namespace common {
namespace env {
std::string env_root = "C://AlphaEngine";

const bool full_screen = false;
const bool vsync_enabled = true;
const double field_of_view = PI / 4.0f;
const float screen_depth = 1000.0f;
const float screen_near = 0.3f;

HWND main_window = 0;
int screen_width = 1920;
int screen_height = 1080;

} // namespace env
} // namespace common