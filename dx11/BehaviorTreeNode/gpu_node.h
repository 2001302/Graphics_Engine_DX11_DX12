#ifndef _GPU_NODE
#define _GPU_NODE

#include "../gpu/graphics_util.h"
#include <behaviortree/behavior_tree_builder.h>

namespace graphics {
class ResolveBufferNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        graphics::GpuCore::Instance().device_context->ResolveSubresource(
            graphics::GpuCore::Instance().resolved_buffer.Get(), 0,
            graphics::GpuCore::Instance().float_buffer.Get(), 0,
            DXGI_FORMAT_R16G16B16A16_FLOAT);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        // Present the rendered scene to the screen.
        if (common::env::vsync_enabled) {
            graphics::GpuCore::Instance().swap_chain->Present(1, 0);
        } else {
            graphics::GpuCore::Instance().swap_chain->Present(0, 0);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace graphics

#endif