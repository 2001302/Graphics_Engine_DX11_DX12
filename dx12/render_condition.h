#ifndef _RENDER_CONDITION
#define _RENDER_CONDITION

#include "common/info.h"
#include "constant_buffer.h"
#include "dynamic_descriptor_heap.h"
#include "gpu_resource.h"
#include "mesh_util.h"
#include "sampler_state.h"
#include "texture_2d.h"
#include "texture_cube.h"

namespace graphics {
enum EnumStageType {
    eInitialize = 0,
    eUpdate = 1,
    eRender = 2,
};

class RenderCondition : public common::IInfo {
  public:
    RenderCondition()
        : delta_time(0.0f), draw_wire(false), light_rotate(false),
          stage_type(EnumStageType::eInitialize) {}

    std::shared_ptr<GpuResourceList> shared_texture; // t10~t16
    std::shared_ptr<SamplerState> shared_sampler;
    ConstantBuffer<GlobalConstants> global_consts;

    float delta_time;
    bool draw_wire;
    bool light_rotate;
    EnumStageType stage_type;
};
} // namespace graphics
#endif
