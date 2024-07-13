#ifndef _BONE
#define _BONE

#include "direct3D.h"

namespace dx11 {

struct Bone {
    int index;
    std::string name;
    int parent;
    DirectX::XMMATRIX transform;
};

} // namespace dx11
#endif
