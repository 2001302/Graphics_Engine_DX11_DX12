#ifndef _GPU_NODE
#define _GPU_NODE

#include "behavior_tree_builder.h"
#include "graphics_util.h"

namespace core {
class ResolveBufferNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        GraphicsCore::Instance().device_context->ResolveSubresource(
            GraphicsCore::Instance().resolved_buffer.Get(), 0,
            GraphicsCore::Instance().float_buffer.Get(), 0,
            DXGI_FORMAT_R16G16B16A16_FLOAT);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        // Present the rendered scene to the screen.
        if (common::Env::Instance().vsync_enabled) {
            GraphicsCore::Instance().swap_chain->Present(1, 0);
        } else {
            GraphicsCore::Instance().swap_chain->Present(0, 0);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif