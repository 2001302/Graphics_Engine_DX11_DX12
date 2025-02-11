#include "shadow_map.h"

namespace graphics
{
GpuResourceList *ShadowMap::GetShadowMap(common::Model *world) {
    ShadowMap *component = nullptr;
    if (world->TryGet(component))
        return component->shadow_texture.get();
    return nullptr;
}
void ShadowMap::Initialize() {
    std::vector<GpuResource *> shadow = {};
    for (int i = 0; i < MAX_LIGHTS; i++) {
        shadow.push_back(std::move(
            DepthBuffer::Create(1024, 1024, DXGI_FORMAT_D24_UNORM_S8_UINT)));
    }
    shadow_texture = std::make_shared<GpuResourceList>(shadow);
}
}