#include "skybox_renderer.h"

namespace graphics 
{
GpuResourceList *SkyboxRenderer::GetSkyboxTexture(common::Model *world) {
    SkyboxRenderer *component = nullptr;
    if (world->TryGet(component))
        return component->skybox_texture.get();
    return nullptr;
}
void SkyboxRenderer::SetTexture(const wchar_t *env_path,
                              const wchar_t *specular_path,
                const wchar_t *diffuse_path, const wchar_t *brdf_path) {
    auto env = TextureCube::Create(env_path);
    auto specular = TextureCube::Create(specular_path);
    auto diffuse = TextureCube::Create(diffuse_path);
    auto brdf = TextureCube::Create(brdf_path, true);

    std::vector<GpuResource *> skybox = {std::move(env), std::move(specular),
                                         std::move(diffuse), std::move(brdf)};
    skybox_texture = std::make_shared<GpuResourceList>(skybox);

    is_black_ = env->IsBlack() && specular->IsBlack() && diffuse->IsBlack() &&
                brdf->IsBlack();
}
}