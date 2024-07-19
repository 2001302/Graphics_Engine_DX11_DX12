#ifndef _CUBEMAP
#define _CUBEMAP

#include "cube_map_shader.h"
#include "node_ui.h"
#include "mesh.h"

namespace engine {
using Microsoft::WRL::ComPtr;

class CubeMap : public common::INodeUi {
  public:
    struct CubeTexture {
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
};
} // namespace dx11
#endif
