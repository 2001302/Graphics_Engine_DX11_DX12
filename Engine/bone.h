#ifndef _BONE
#define _BONE

#include "graphics_manager.h"

namespace engine {

struct Bone {
    int index;
    std::string name;
    int parent;
    DirectX::XMMATRIX transform;
};

} // namespace dx11
#endif
