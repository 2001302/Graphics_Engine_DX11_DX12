#ifndef _CUBEMAP
#define _CUBEMAP

#include "entity.h"
#include "common_struct.h"
#include "cube_map_shader.h"

namespace Engine {
using Microsoft::WRL::ComPtr;

class CubeMap : public IEntity{
  public:
    CubeMap();
    ~CubeMap();

    int GetIndexCount();

    DirectX::SimpleMath::Matrix transform;
    std::shared_ptr<Mesh> mesh;
    ComPtr<ID3D11ShaderResourceView> diffuseResView;
    ComPtr<ID3D11ShaderResourceView> specularResView;
    std::shared_ptr<CubeMapShaderSource> cube_map_shader;
};
} // namespace Engine
#endif \
