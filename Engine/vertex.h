#ifndef _VERTEX
#define _VERTEX

#include "graphics_manager.h"

namespace engine {

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
    Vector3 tangent;
};

} // namespace dx11
#endif
