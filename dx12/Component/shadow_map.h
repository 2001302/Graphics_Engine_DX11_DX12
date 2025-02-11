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
    static GpuResourceList *GetShadowMap(common::Model *world);
    void Initialize();

  private:
    std::shared_ptr<GpuResourceList> shadow_texture; // t14~t16
};
} // namespace graphics
#endif