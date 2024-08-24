#ifndef _GPU_NODE
#define _GPU_NODE

#include "../foundation/behavior_tree_builder.h"
#include "../graphics/graphics_util.h"

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

class BeginRenderNode : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        black_board->conditions->command_pool->Open();

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle =
            dx12::GpuCore::Instance().GetHandleRTV();

        const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        black_board->conditions->command_pool->Get(0)
            ->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
            dx12::GpuCore::Instance()
                .heap_DSV->GetCPUDescriptorHandleForHeapStart());
        black_board->conditions->command_pool->Get(0)
            ->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0,
                                    0, nullptr);

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

// class ResolveBufferNode : public foundation::BehaviorActionNode {
//     foundation::EnumBehaviorTreeStatus OnInvoke() override {
//
//          dx12::GpuCore::Instance().commandList->ResolveSubresource(
//              dx12::GpuCore::Instance().resolved_buffer.Get(), 0,
//              dx12::GpuCore::Instance().float_buffer.Get(), 0,
//              DXGI_FORMAT_R16G16B16A16_FLOAT);
//
//         return foundation::EnumBehaviorTreeStatus::eSuccess;
//     }
// };

} // namespace core

#endif
