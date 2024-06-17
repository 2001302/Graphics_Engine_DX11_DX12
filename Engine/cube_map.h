#ifndef _CUBEMAP
#define _CUBEMAP

#include "common_struct.h"
#include "entity.h"

namespace Engine {
using Microsoft::WRL::ComPtr;

class CubeMap : public IEntity{
  public:
    CubeMap();
    ~CubeMap();

    int GetIndexCount();

    DirectX::SimpleMath::Matrix transform;
    std::vector<std::shared_ptr<Mesh>> meshes;
    // cube map shader
};
} // namespace Engine
#endif \
