#ifndef _GPU_NODE
#define _GPU_NODE

#include "foundation/behavior_tree_builder.h"
#include "graphics/graphics_util.h"

namespace core {
class GpuInitializeNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        dx12::GpuCore::Instance().InitializeGPU();
        black_board->conditions->command_pool->InitializeAll();

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class BeginInitNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {
        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        black_board->conditions->command_pool->Open();
        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class EndInitNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {
        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        black_board->conditions->command_pool->Close();
        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class BeginRenderNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);
        auto command_list = black_board->conditions->command_pool->Get(0);

        black_board->conditions->command_pool->Open();

        // back buffer
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance()
                .resource_FLIP[dx12::GpuCore::Instance().frame_index]
                .Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
        command_list->ResourceBarrier(1, &barrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE handle_back_buffer =
            dx12::GpuCore::Instance().GetHandleFLIP();
        const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        command_list->OMSetRenderTargets(1, &handle_back_buffer, true, nullptr);
        command_list->ClearRenderTargetView(handle_back_buffer, clearColor, 0,
                                            nullptr);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance()
                .resource_FLIP[dx12::GpuCore::Instance().frame_index]
                .Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
        command_list->ResourceBarrier(1, &barrier);

        // HDR
        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance().resource_HDR.Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);
        command_list->ResourceBarrier(1, &barrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE handle_HDR =
            dx12::GpuCore::Instance().GetHandleHDR();
        command_list->OMSetRenderTargets(1, &handle_HDR, true, nullptr);
        command_list->ClearRenderTargetView(handle_HDR, clearColor, 0, nullptr);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance().resource_HDR.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON);
        command_list->ResourceBarrier(1, &barrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
            dx12::GpuCore::Instance()
                .heap_DSV->GetCPUDescriptorHandleForHeapStart());
        command_list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH,
                                            1.0f, 0, 0, nullptr);

        ID3D12DescriptorHeap *descriptorHeaps[] = {
            black_board->conditions->sampler_heap.Get(),
            &black_board->conditions->gpu_heap->GetDescriptorHeap()};

        black_board->conditions->command_pool->Get(0)->SetDescriptorHeaps(
            _countof(descriptorHeaps), descriptorHeaps);

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class EndRenderNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        black_board->conditions->command_pool->Close();

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public foundation::BehaviorActionNode {

    void WaitForPreviousFrame() {
        // Signal and increment the fence value.
        const UINT64 fence = dx12::GpuCore::Instance().fence_value;
        dx12::GpuCore::Instance().command_queue->Signal(
            dx12::GpuCore::Instance().fence.Get(), fence);
        dx12::GpuCore::Instance().fence_value++;

        // Wait until the previous frame is finished.
        if (dx12::GpuCore::Instance().fence->GetCompletedValue() < fence) {
            dx12::GpuCore::Instance().fence->SetEventOnCompletion(
                fence, dx12::GpuCore::Instance().fence_event);
            WaitForSingleObject(dx12::GpuCore::Instance().fence_event,
                                INFINITE);
        }

        dx12::GpuCore::Instance().frame_index =
            dx12::GpuCore::Instance().swap_chain->GetCurrentBackBufferIndex();
    }

    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        dx12::GpuCore::Instance().swap_chain->Present(1, 0);
        WaitForPreviousFrame();

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

class ResolveBuffer : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);
        auto command_list = black_board->conditions->command_pool->Get(0);

        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance().resource_HDR.Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
        command_list->ResourceBarrier(1, &barrier);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance().resource_LDR.Get(),
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RESOLVE_DEST);
        command_list->ResourceBarrier(1, &barrier);

        // resolve
        command_list->ResolveSubresource(
            dx12::GpuCore::Instance().resource_LDR.Get(), 0,
            dx12::GpuCore::Instance().resource_HDR.Get(), 0,
            DXGI_FORMAT_R16G16B16A16_FLOAT);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance().resource_HDR.Get(),
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_COMMON);
        command_list->ResourceBarrier(1, &barrier);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            dx12::GpuCore::Instance().resource_LDR.Get(),
            D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_COMMON);
        command_list->ResourceBarrier(1, &barrier);

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace core

#endif
