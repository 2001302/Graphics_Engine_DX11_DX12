#ifndef _BONE
#define _BONE

#include "graphics_context.h"

namespace dx11 {

struct Bone {
    int index;
    std::string name;
    int parent;
    DirectX::XMMATRIX transform;
};

} // namespace dx11
#endif
