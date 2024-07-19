#ifndef _VERTEX
#define _VERTEX

#include "direct3D.h"

namespace dx11 {

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
    Vector3 tangent;
};

} // namespace dx11
#endif
