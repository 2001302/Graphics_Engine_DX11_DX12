#include "graphics_pso.h"

namespace core {

void GraphicsPSO::operator=(const GraphicsPSO &pso) {
    vertex_shader = pso.vertex_shader;
    pixel_shader = pso.pixel_shader;
    hull_shader = pso.hull_shader;
    domain_shader = pso.domain_shader;
    geometry_shader = pso.geometry_shader;
    input_layout = pso.input_layout;
    blend_state = pso.blend_state;
    depth_stencil_state = pso.depth_stencil_state;
    rasterizer_state = pso.rasterizer_state;
    stencil_ref = pso.stencil_ref;
    for (int i = 0; i < 4; i++)
        blend_factor[i] = pso.blend_factor[i];
    primitive_topology = pso.primitive_topology;
}

void GraphicsPSO::SetBlendFactor(const float blendFactor[4]) {
    memcpy(blend_factor, blendFactor, sizeof(float) * 4);
}

} // namespace engine