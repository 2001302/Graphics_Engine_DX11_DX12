#ifndef _RENDER_CONDITION
#define _RENDER_CONDITION

#include "foundation/info.h"
#include "graphics/command_pool.h"
#include "graphics/constant_buffer.h"
#include "graphics/gpu_buffer.h"
#include "graphics/gpu_heap.h"
#include "graphics/graphics_util.h"
#include "graphics/texture_2d.h"
#include "graphics/texture_cube.h"

namespace core {
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
        command_pool = std::make_shared<dx12::CommandPool>();
    }

    std::shared_ptr<dx12::CommandPool> command_pool;
    std::shared_ptr<dx12::GpuHeap> gpu_heap;
    std::shared_ptr<dx12::GpuBufferList> shared_texture; // t10~t16
    dx12::ConstantBuffer<dx12::GlobalConstants> global_consts;

    float dt;
    bool draw_wire;
    bool light_rotate;
    EnumStageType stage_type;
};
} // namespace core
#endif
