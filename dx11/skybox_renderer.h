#ifndef _SKYBOX_RENDERER
#define _SKYBOX_RENDERER

#include "mesh_renderer.h"

namespace graphics {
struct SkyboxRenderer : public MeshRenderer {
  public:
    using MeshRenderer::MeshRenderer;
    ComPtr<ID3D11ShaderResourceView> env_SRV;
    ComPtr<ID3D11ShaderResourceView> irradiance_SRV;
    ComPtr<ID3D11ShaderResourceView> specular_SRV;
    ComPtr<ID3D11ShaderResourceView> brdf_SRV;
};
} // namespace engine
#endif
