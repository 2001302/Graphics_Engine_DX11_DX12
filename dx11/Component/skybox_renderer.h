#ifndef _SKYBOX_RENDERER
#define _SKYBOX_RENDERER

#include "mesh_renderer.h"

namespace graphics {
struct SkyboxRenderer : public MeshRenderer {
  public:
    using MeshRenderer::MeshRenderer;

    void CreateCubeMap(const wchar_t *env, const wchar_t *specular,
                       const wchar_t *irradiance, const wchar_t *brdf) {

        if (std::filesystem::exists(env) && std::filesystem::exists(specular) &&
            std::filesystem::exists(irradiance) &&
            std::filesystem::exists(brdf)) {

            graphics::Util::CreateDDSTexture(env, true, env_SRV);
            graphics::Util::CreateDDSTexture(specular, true, specular_SRV);
            graphics::Util::CreateDDSTexture(irradiance, true, irradiance_SRV);
            graphics::Util::CreateDDSTexture(brdf, true, brdf_SRV);
        }
    }

    ComPtr<ID3D11ShaderResourceView> env_SRV;
    ComPtr<ID3D11ShaderResourceView> irradiance_SRV;
    ComPtr<ID3D11ShaderResourceView> specular_SRV;
    ComPtr<ID3D11ShaderResourceView> brdf_SRV;
};
} // namespace graphics
#endif
