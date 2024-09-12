#ifndef _GPU_NODE
#define _GPU_NODE

#include "foundation/behavior_tree_builder.h"
#include "graphics_util.h"

namespace graphics {
class GpuInitializeNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        GpuCore::Instance().Initialize();

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class BeginInitNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        // GpuCore::Instance().GetCommandMgr()->Queue()

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class EndInitNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        // GpuCore::Instance().GetCommandMgr()->Queue()

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class BeginRenderNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto command_manager = GpuCore::Instance().GetCommand();

        command_manager.GraphicsList()->Reset();

        //     command_manager->GraphicsList().TransitionResource(
        // GpuCore::Instance().GetBackBuffer(),
        //         D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        //     command_manager->GraphicsList().ClearRenderTargetView(
        //         GpuCore::Instance().GetBackBuffer());
        //     command_manager->GraphicsList().SetRenderTargetView(
        //         GpuCore::Instance().GetBackBuffer());

        command_manager.GraphicsList()->SetDescriptorHeaps(
            std::vector{GpuCore::Instance().GetHeap().GetRTVHeap(),
                        GpuCore::Instance().GetHeap().GetViewHeap(),
                        GpuCore::Instance().GetHeap().GetSamplerHeap(),
                        GpuCore::Instance().GetHeap().GetDSVHeap()});

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

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class EndRenderNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        GpuCore::Instance().GetCommand().GraphicsList()->Close();

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class GpuFenceNode : public foundation::BehaviorActionNode {

    void WaitForPreviousFrame() {
        //// Signal and increment the fence value.
        // const UINT64 fence = GpuDevice::Get().fence_value;
        // GpuDevice::Get().command_queue->Signal(
        //     GpuDevice::Get().fence.Get(), fence);
        // GpuDevice::Get().fence_value++;

        //// Wait until the previous frame is finished.
        // if (GpuDevice::Get().fence->GetCompletedValue() < fence) {
        //     GpuDevice::Get().fence->SetEventOnCompletion(
        //         fence, GpuDevice::Get().fence_event);
        //     WaitForSingleObject(GpuDevice::Get().fence_event, INFINITE);
        // }

        // GpuDevice::Get().frame_index =
        //     GpuDevice::Get().swap_chain->GetCurrentBackBufferIndex();
    }

    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        WaitForPreviousFrame();

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public foundation::BehaviorActionNode {

    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        GpuCore::Instance().GetSwapChain()->Present(1, 0);

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class ResolveBuffer : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

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

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace graphics

#endif