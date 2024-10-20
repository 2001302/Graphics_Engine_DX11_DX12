#ifndef _SKYBOX_RENDERER
#define _SKYBOX_RENDERER

#include "mesh_renderer.h"
#include "mesh_util.h"

namespace graphics {
class SkyboxRenderer : public MeshRenderer {
  public:
    SkyboxRenderer(){};
    ~SkyboxRenderer(){};
    static GpuResourceList *GetSkyboxTexture(common::Model *world) {
        SkyboxRenderer *component = nullptr;
        if (world->TryGet(component))
            return component->skybox_texture.get();
        return nullptr;
    }
    void Initialize(const std::vector<MeshData> &meshes,
                    const wchar_t *env_path = nullptr,
                    const wchar_t *specular_path = nullptr,
                    const wchar_t *diffuse_path = nullptr,
                    const wchar_t *brdf_path = nullptr) {
        MeshRenderer::Initialize(meshes);
        SetTexture(env_path, specular_path, diffuse_path, brdf_path);
    }

  private:
    void SetTexture(const wchar_t *env_path, const wchar_t *specular_path,
                    const wchar_t *diffuse_path, const wchar_t *brdf_path) {
        auto env = TextureCube::Create(env_path);
        auto specular = TextureCube::Create(specular_path);
        auto diffuse = TextureCube::Create(diffuse_path);
        auto brdf = TextureCube::Create(brdf_path, true);

        std::vector<GpuResource *> skybox = {
            std::move(env), std::move(specular), std::move(diffuse),
            std::move(brdf)};
        skybox_texture = std::make_shared<GpuResourceList>(skybox);
    }

    std::shared_ptr<GpuResourceList> skybox_texture; // t10~t13
};
} // namespace graphics
#endif