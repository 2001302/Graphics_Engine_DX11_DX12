#ifndef _GPU_NODE
#define _GPU_NODE

#include "foundation/behavior_tree_builder.h"
#include "graphics_util.h"

namespace graphics {
class GpuInitializeNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        //GpuDevice::Get().InitializeGPU();
        //black_board->conditions->command_pool->InitializeAll();

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class BeginInitNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {
        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        //black_board->conditions->command_pool->Open();
        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class EndInitNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {
        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        //black_board->conditions->command_pool->Close();
        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class BeginRenderNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        //auto command_list = black_board->conditions->command_pool->Get(0);

        //black_board->conditions->command_pool->Open();

        //// back buffer
        //auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //    GpuDevice::Get()
        //        .resource_FLIP[GpuDevice::Get().frame_index]
        //        .Get(),
        //    D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
        //command_list->ResourceBarrier(1, &barrier);

        //CD3DX12_CPU_DESCRIPTOR_HANDLE handle_back_buffer =
        //    GpuDevice::Get().GetHandleFLIP();
        //const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        //command_list->OMSetRenderTargets(1, &handle_back_buffer, true, nullptr);
        //command_list->ClearRenderTargetView(handle_back_buffer, clearColor, 0,
        //                                    nullptr);

        //barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //    GpuDevice::Get()
        //        .resource_FLIP[GpuDevice::Get().frame_index]
        //        .Get(),
        //    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
        //command_list->ResourceBarrier(1, &barrier);

        //// HDR
        //barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //    GpuDevice::Get().resource_HDR.Get(), D3D12_RESOURCE_STATE_COMMON,
        //    D3D12_RESOURCE_STATE_RENDER_TARGET);
        //command_list->ResourceBarrier(1, &barrier);

        //CD3DX12_CPU_DESCRIPTOR_HANDLE handle_HDR =
        //    GpuDevice::Get().GetHandleHDR();
        //command_list->OMSetRenderTargets(1, &handle_HDR, true, nullptr);
        //command_list->ClearRenderTargetView(handle_HDR, clearColor, 0, nullptr);

        //barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //    GpuDevice::Get().resource_HDR.Get(),
        //    D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
        //command_list->ResourceBarrier(1, &barrier);

        //CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
        //    GpuDevice::Get().heap_DSV->GetCPUDescriptorHandleForHeapStart());
        //command_list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH,
        //                                    1.0f, 0, 0, nullptr);

        //ID3D12DescriptorHeap *descriptorHeaps[] = {
        //    &black_board->conditions->sampler_heap->Get(),
        //    &black_board->conditions->gpu_heap->Get()};

        //black_board->conditions->command_pool->Get(0)->SetDescriptorHeaps(
        //    _countof(descriptorHeaps), descriptorHeaps);

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class EndRenderNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        //black_board->conditions->command_pool->Close();

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class GpuFenceNode : public foundation::BehaviorActionNode {

    void WaitForPreviousFrame() {
        //// Signal and increment the fence value.
        //const UINT64 fence = GpuDevice::Get().fence_value;
        //GpuDevice::Get().command_queue->Signal(
        //    GpuDevice::Get().fence.Get(), fence);
        //GpuDevice::Get().fence_value++;

        //// Wait until the previous frame is finished.
        //if (GpuDevice::Get().fence->GetCompletedValue() < fence) {
        //    GpuDevice::Get().fence->SetEventOnCompletion(
        //        fence, GpuDevice::Get().fence_event);
        //    WaitForSingleObject(GpuDevice::Get().fence_event, INFINITE);
        //}

        //GpuDevice::Get().frame_index =
        //    GpuDevice::Get().swap_chain->GetCurrentBackBufferIndex();
    }

    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        WaitForPreviousFrame();

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public foundation::BehaviorActionNode {

    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        //GpuDevice::Get().swap_chain->Present(1, 0);

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class ResolveBuffer : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        //auto command_list = black_board->conditions->command_pool->Get(0);

        //auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //    GpuDevice::Get().resource_HDR.Get(), D3D12_RESOURCE_STATE_COMMON,
        //    D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
        //command_list->ResourceBarrier(1, &barrier);

        //barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //    GpuDevice::Get().resource_LDR.Get(), D3D12_RESOURCE_STATE_COMMON,
        //    D3D12_RESOURCE_STATE_RESOLVE_DEST);
        //command_list->ResourceBarrier(1, &barrier);

        //// resolve
        //command_list->ResolveSubresource(GpuDevice::Get().resource_LDR.Get(),
        //                                 0,
        //                                 GpuDevice::Get().resource_HDR.Get(),
        //                                 0, DXGI_FORMAT_R16G16B16A16_FLOAT);

        //barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //    GpuDevice::Get().resource_HDR.Get(),
        //    D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_COMMON);
        //command_list->ResourceBarrier(1, &barrier);

        //barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        //    GpuDevice::Get().resource_LDR.Get(),
        //    D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_COMMON);
        //command_list->ResourceBarrier(1, &barrier);

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace core

#endif
