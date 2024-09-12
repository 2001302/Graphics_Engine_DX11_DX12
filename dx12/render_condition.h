#ifndef _RENDER_CONDITION
#define _RENDER_CONDITION

#include "constant_buffer.h"
#include "foundation/info.h"
#include "gpu_resource.h"
#include "descriptor_heap.h"
#include "graphics_util.h"
#include "texture_2d.h"
#include "texture_cube.h"

namespace graphics {
enum EnumStageType {
    eInitialize = 0,
    eUpdate = 1,
    eRender = 2,
};

class RenderCondition : public foundation::IInfo {
  public:
    RenderCondition()
        : dt(0.0f), draw_wire(false), light_rotate(false),
          stage_type(EnumStageType::eInitialize) {
    }

    std::shared_ptr<GpuResourceList> shared_texture; // t10~t16
    ConstantBuffer<GlobalConstants> global_consts;

    float dt;
    bool draw_wire;
    bool light_rotate;
    EnumStageType stage_type;
};
} // namespace core
#endif