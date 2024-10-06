#ifndef _RENDER_CONDITION
#define _RENDER_CONDITION

#include "constant_buffer.h"
#include "dynamic_descriptor_heap.h"
#include "gpu_resource.h"
#include "mesh_util.h"
#include "sampler_state.h"
#include "texture_2d.h"
#include "texture_cube.h"
#include <info.h>

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
          stage_type(EnumStageType::eInitialize), shared_sampler(0) {}
    ~RenderCondition() {

        if (skybox_texture) {
            delete skybox_texture;
            skybox_texture = 0;
        }
        if (shared_sampler) {
            delete shared_sampler;
            shared_sampler = 0;
        }
    }

    GpuResourceList *skybox_texture; // t10~t13
    GpuResourceList *shadow_texture; // t14~t16
    SamplerState *shared_sampler;
    ConstantBuffer<GlobalConstants> global_consts;

    float delta_time;
    bool draw_wire;
    bool light_rotate;
    EnumStageType stage_type;
};
} // namespace graphics
#endif
