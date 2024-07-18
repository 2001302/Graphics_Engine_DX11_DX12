#ifndef _VERTEX
#define _VERTEX

#include "graphics_manager.h"

namespace dx11 {

struct Vertex {
    Vector3 position;
    Vector3 normalModel;
    Vector2 texcoord;
    Vector3 tangentModel;
};

} // namespace dx11
#endif
