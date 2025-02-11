#ifndef _SKYBOX_RENDERER
#define _SKYBOX_RENDERER

#include "mesh_renderer.h"
#include "../util/mesh_util.h"

namespace graphics {
class SkyboxRenderer : public MeshRenderer {
  public:
    SkyboxRenderer() : is_black_(false){};
    ~SkyboxRenderer(){};
    void Initialize(const std::vector<MeshData> &meshes,
                    const wchar_t *env_path = nullptr,
                    const wchar_t *specular_path = nullptr,
                    const wchar_t *diffuse_path = nullptr,
                    const wchar_t *brdf_path = nullptr) {
        MeshRenderer::Initialize(meshes);
        SetTexture(env_path, specular_path, diffuse_path, brdf_path);
    }
    static GpuResourceList *GetSkyboxTexture(common::Model *world);
    bool IsBlack() { return is_black_; }

  private:
    void SetTexture(const wchar_t *env_path, const wchar_t *specular_path,
                    const wchar_t *diffuse_path, const wchar_t *brdf_path);

    std::shared_ptr<GpuResourceList> skybox_texture; // t10~t13
    bool is_black_;
};
} // namespace graphics
#endif