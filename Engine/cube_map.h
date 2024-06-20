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
    ComPtr<ID3D11ShaderResourceView> diffuse_resource_view;
    ComPtr<ID3D11ShaderResourceView> specular_resource_view;
    std::shared_ptr<CubeMapShaderSource> cube_map_shader_source;
};
} // namespace Engine
#endif \
