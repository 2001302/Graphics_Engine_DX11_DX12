#ifndef _GPU_NODE
#define _GPU_NODE

#include "behavior_tree_builder.h"
#include "graphics_util.h"

namespace core {
class ResolveBufferNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        dx11::GpuCore::Instance().device_context->ResolveSubresource(
            dx11::GpuCore::Instance().resolved_buffer.Get(), 0,
            dx11::GpuCore::Instance().float_buffer.Get(), 0,
            DXGI_FORMAT_R16G16B16A16_FLOAT);

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        // Present the rendered scene to the screen.
        if (foundation::Env::Instance().vsync_enabled) {
            dx11::GpuCore::Instance().swap_chain->Present(1, 0);
        } else {
            dx11::GpuCore::Instance().swap_chain->Present(0, 0);
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace core

#endif