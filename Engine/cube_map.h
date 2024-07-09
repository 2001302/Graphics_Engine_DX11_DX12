#ifndef _CUBEMAP
#define _CUBEMAP

#include "entity.h"
#include "common_struct.h"
#include "cube_map_shader.h"
#include "game_object.h"

namespace Engine {
using Microsoft::WRL::ComPtr;

class CubeMap : public GameObject {
  public:
    CubeMap();
    ~CubeMap();

    int GetIndexCount();

    DirectX::SimpleMath::Matrix transform;
    std::shared_ptr<Mesh> mesh;
    ComPtr<ID3D11ShaderResourceView> env_SRV;
    ComPtr<ID3D11ShaderResourceView> specular_SRV;   // Radiance
    ComPtr<ID3D11ShaderResourceView> irradiance_SRV; // Diffuse
    ComPtr<ID3D11ShaderResourceView> brdf_SRV;       // BRDF LookUpTable
    std::shared_ptr<CubeMapShaderSource> cube_map_shader_source;
};
} // namespace Engine
#endif \
