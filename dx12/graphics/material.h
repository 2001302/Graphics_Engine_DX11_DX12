#ifndef _MATERIAL
#define _MATERIAL

#include "graphics_util.h"

namespace core {

struct Material {
    Vector3 ambient = Vector3(0.1f);  // 12
    float shininess = 1.0f;           // 4
    Vector3 diffuse = Vector3(0.5f);  // 12
    float dummy1;                     // 4
    Vector3 specular = Vector3(0.5f); // 12
    float dummy2;                     // 4
};

} // namespace dx11
#endif
