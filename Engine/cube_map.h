#ifndef _CUBEMAP
#define _CUBEMAP

#include "common_struct.h"
#include "cube_map_shader.h"
#include "entity.h"
#include "game_object.h"

namespace Engine {
using Microsoft::WRL::ComPtr;

class CubeMap : public GameObject {
  public:
    struct CubeTexture : public Texture {
        ComPtr<ID3D11ShaderResourceView> env_SRV;
        ComPtr<ID3D11ShaderResourceView> specular_SRV;   // Radiance
        ComPtr<ID3D11ShaderResourceView> irradiance_SRV; // Diffuse
        ComPtr<ID3D11ShaderResourceView> brdf_SRV;       // BRDF LookUpTable
    };
    CubeMap();
    ~CubeMap();

    int GetIndexCount();

    DirectX::SimpleMath::Matrix transform;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<CubeTexture> texture;
    std::shared_ptr<CubeMapShaderSource> cube_map_shader_source;

};
} // namespace Engine
#endif
