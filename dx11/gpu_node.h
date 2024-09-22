#ifndef _GPU_NODE
#define _GPU_NODE

#include "behavior_tree_builder.h"
#include "graphics_util.h"

namespace graphics {
class ResolveBufferNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        graphics::GpuCore::Instance().device_context->ResolveSubresource(
            graphics::GpuCore::Instance().resolved_buffer.Get(), 0,
            graphics::GpuCore::Instance().float_buffer.Get(), 0,
            DXGI_FORMAT_R16G16B16A16_FLOAT);

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        // Present the rendered scene to the screen.
        if (foundation::Env::Instance().vsync_enabled) {
            graphics::GpuCore::Instance().swap_chain->Present(1, 0);
        } else {
            graphics::GpuCore::Instance().swap_chain->Present(0, 0);
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace core

#endif