#ifndef _PROJECTION
#define _PROJECTION

#include "graphics_manager.h"

namespace dx11 {

struct Projection {
    float projection_fov_angle_y = 70.0f;
    float near_z = 0.01f;
    float far_z = 100.0f;
};

} // namespace dx11
#endif
