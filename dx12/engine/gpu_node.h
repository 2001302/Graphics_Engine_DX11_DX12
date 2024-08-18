#ifndef _GPU_NODE
#define _GPU_NODE

#include "../foundation/behavior_tree_builder.h"
#include "../graphics/graphics_util.h"

namespace core {
class ResolveBufferNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        // dx12::GpuCore::Instance().commandList->ResolveSubresource(
        //     dx12::GpuCore::Instance().resolved_buffer.Get(), 0,
        //     dx12::GpuCore::Instance().float_buffer.Get(), 0,
        //     DXGI_FORMAT_R16G16B16A16_FLOAT);

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class StartRenderingNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        // reset the command list and allocator
        dx12::GpuCore::Instance().commandAllocator->Reset();
        dx12::GpuCore::Instance().commandList->Reset(
            dx12::GpuCore::Instance().commandAllocator.Get(), nullptr);

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance()
                .renderTargets[dx12::GpuCore::Instance().frameIndex]
                .Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        dx12::GpuCore::Instance().commandList->ResourceBarrier(1, &barrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
            dx12::GpuCore::Instance()
                .rtvHeap->GetCPUDescriptorHandleForHeapStart(),
            dx12::GpuCore::Instance().frameIndex,
            dx12::GpuCore::Instance().rtvDescriptorSize);
        const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        dx12::GpuCore::Instance().commandList->ClearRenderTargetView(
            rtvHandle, clearColor, 0, nullptr);
        dx12::GpuCore::Instance().commandList->OMSetRenderTargets(
            1, &rtvHandle, false, nullptr);

        dx12::GpuCore::Instance().commandList->SetDescriptorHeaps(
            1, dx12::GpuCore::Instance().srvHeap.GetAddressOf());

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public foundation::BehaviorActionNode {

    void WaitForPreviousFrame() {
        // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST
        // PRACTICE. This is code implemented as such for simplicity. The
        // D3D12HelloFrameBuffering sample illustrates how to use fences for
        // efficient resource usage and to maximize GPU utilization.

        // Signal and increment the fence value.
        const UINT64 fence = dx12::GpuCore::Instance().fenceValue;
        dx12::GpuCore::Instance().command_queue->Signal(
            dx12::GpuCore::Instance().fence.Get(), fence);
        dx12::GpuCore::Instance().fenceValue++;

        // Wait until the previous frame is finished.
        if (dx12::GpuCore::Instance().fence->GetCompletedValue() < fence) {
            dx12::GpuCore::Instance().fence->SetEventOnCompletion(
                fence, dx12::GpuCore::Instance().fenceEvent);
            WaitForSingleObject(dx12::GpuCore::Instance().fenceEvent, INFINITE);
        }

        dx12::GpuCore::Instance().frameIndex =
            dx12::GpuCore::Instance().swap_chain->GetCurrentBackBufferIndex();
    }

    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        // preset the render target as the back buffer
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance()
                .renderTargets[dx12::GpuCore::Instance().frameIndex]
                .Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        dx12::GpuCore::Instance().commandList->ResourceBarrier(1, &barrier);

        // close the command list after all commands have been added
        dx12::GpuCore::Instance().commandList->Close();
        // execute the command list
        dx12::GpuCore::Instance().command_queue->ExecuteCommandLists(
            1, CommandListCast(
                   dx12::GpuCore::Instance().commandList.GetAddressOf()));
        dx12::GpuCore::Instance().swap_chain->Present(1, 0);
        WaitForPreviousFrame();

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace core

#endif