#ifndef _SHAODW_MAP
#define _SHAODW_MAP

#include "../gpu/buffer/gpu_resource.h"
#include "../util/mesh_util.h"
#include <structure/model.h>

namespace graphics {
class ShadowMap : public common::Component {
  public:
    ShadowMap(){};
    ~ShadowMap(){};
    static GpuResourceList *GetShadowMap(common::Model *world) {
        ShadowMap *component = nullptr;
        if (world->TryGet(component))
            return component->shadow_texture.get();
        return nullptr;
    }
    void Initialize() {
        std::vector<GpuResource *> shadow = {};
        for (int i = 0; i < MAX_LIGHTS; i++) {
            shadow.push_back(std::move(DepthBuffer::Create(
                1024, 1024, DXGI_FORMAT_D24_UNORM_S8_UINT)));
        }
        shadow_texture = std::make_shared<GpuResourceList>(shadow);
    }

  private:
    std::shared_ptr<GpuResourceList> shadow_texture; // t14~t16
};
} // namespace graphics
#endif