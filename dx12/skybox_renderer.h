#ifndef _SKYBOX_RENDERER
#define _SKYBOX_RENDERER

#include "mesh_renderer.h"
#include "mesh_util.h"

namespace graphics {
class SkyboxRenderer : public MeshRenderer {
  public:
    SkyboxRenderer(){};
    ~SkyboxRenderer(){};
    static GpuResourceList *GetSkyboxTexture(common::Model *skybox) {
        SkyboxRenderer *component = nullptr;
        if (skybox->TryGet(component))
            return component->skybox_texture.get();
        return nullptr;
    }
    void SetSkyboxTexture(const wchar_t *env_path, const wchar_t *specular_path,
                          const wchar_t *diffuse_path,
                          const wchar_t *brdf_path) {
        auto env = TextureCube::Create(
            L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds");
        auto specular = TextureCube::Create(
            L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds");
        auto diffuse = TextureCube::Create(
            L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds");
        auto brdf = TextureCube::Create(
            L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds");

        std::vector<GpuResource *> skybox = {
            std::move(env), std::move(specular), std::move(diffuse),
            std::move(brdf)};
        skybox_texture = std::make_shared<GpuResourceList>(skybox);
    }

  private:
    std::shared_ptr<GpuResourceList> skybox_texture; // t10~t13
};
} // namespace graphics
#endif