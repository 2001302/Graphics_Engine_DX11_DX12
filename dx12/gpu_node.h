#ifndef _GPU_NODE
#define _GPU_NODE

#include "common/behavior_tree_builder.h"
#include "graphics_util.h"

namespace graphics {
class GpuInitializeNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        GpuCore::Instance().Initialize();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class BeginInitNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        // GpuCore::Instance().GetCommandMgr()->Queue()

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class EndInitNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        // GpuCore::Instance().GetCommandMgr()->Queue()

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class BeginRenderNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto command_manager = GpuCore::Instance().GetCommand();

        command_manager.Begin(D3D12_COMMAND_LIST_TYPE_DIRECT);

        command_manager.GraphicsList()->SetDescriptorHeaps(
            std::vector{GpuCore::Instance().GetHeap().View(),
                        GpuCore::Instance().GetHeap().Sampler()});

        command_manager.GraphicsList()->TransitionResource(
            GpuCore::Instance().GetBuffers().hdr_buffer,
            D3D12_RESOURCE_STATE_RENDER_TARGET, true);

        command_manager.GraphicsList()->ClearRenderTargetView(
            GpuCore::Instance().GetBuffers().hdr_buffer);

        command_manager.GraphicsList()->ClearDepthStencilView(
            GpuCore::Instance().GetBuffers().dsv_buffer);

        command_manager.GraphicsList()->SetRenderTargetView(
            GpuCore::Instance().GetBuffers().hdr_buffer,
            GpuCore::Instance().GetBuffers().dsv_buffer);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class EndRenderNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        GpuCore::Instance().GetCommand().GraphicsList()->TransitionResource(
            GpuCore::Instance().GetDisplay(), D3D12_RESOURCE_STATE_PRESENT,
            true);

        GpuCore::Instance().GetCommand().Finish(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                true);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class GpuFenceNode : public common::BehaviorActionNode {

    common::EnumBehaviorTreeStatus OnInvoke() override {

        // Not implemented

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public common::BehaviorActionNode {

    common::EnumBehaviorTreeStatus OnInvoke() override {

        GpuCore::Instance().GetSwapChain()->Present(1, 0);
        GpuCore::Instance().GetDisplay()->MoveToNext();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class ResolveBuffer : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto command_manager = GpuCore::Instance().GetCommand();

        command_manager.GraphicsList()->TransitionResource(
            GpuCore::Instance().GetBuffers().hdr_buffer,
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE, true);

        command_manager.GraphicsList()->TransitionResource(
            GpuCore::Instance().GetBuffers().ldr_buffer,
            D3D12_RESOURCE_STATE_RESOLVE_DEST, true);

        command_manager.GraphicsList()->ResolveSubresource(
            GpuCore::Instance().GetBuffers().ldr_buffer,
            GpuCore::Instance().GetBuffers().hdr_buffer,
            DXGI_FORMAT_R16G16B16A16_FLOAT);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace graphics

#endif
