#ifndef _RENDER_CONDITION
#define _RENDER_CONDITION

#include "command_pool.h"
#include "constant_buffer.h"
#include "foundation/info.h"
#include "gpu_resource.h"
#include "gpu_heap.h"
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
        command_pool = std::make_shared<CommandPool>();
    }

    std::shared_ptr<CommandPool> command_pool;
    std::shared_ptr<GpuHeap> gpu_heap;
    std::shared_ptr<GpuHeap> sampler_heap;
    std::shared_ptr<GpuResourceList> shared_texture; // t10~t16
    ConstantBuffer<GlobalConstants> global_consts;

    float dt;
    bool draw_wire;
    bool light_rotate;
    EnumStageType stage_type;
};
} // namespace core
#endif
