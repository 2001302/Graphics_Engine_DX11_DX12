#ifndef _COMMAND_POOL
#define _COMMAND_POOL

#define NUM_COMAAND_LIST 3

#include "graphics_core.h"

namespace dx12 {
class CommandPool {
  public:
    CommandPool(){};
    void InitializeAll() {
        for (int i = 0; i < NUM_COMAAND_LIST; i++) {
            ThrowIfFailed(GpuCore::Instance().device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&command_allocator[i])));
            ThrowIfFailed(GpuCore::Instance().device->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator[i].Get(),
                nullptr, IID_PPV_ARGS(&command_lists[i])));
            command_lists[i]->Close();
        }
    }
    void Open(int begin, int end) {
        for (int i = begin; i < end; i++) {
            command_allocator[i]->Reset();
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                dx12::GpuCore::Instance()
                    .render_targets[dx12::GpuCore::Instance().frame_index]
                    .Get(),
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RENDER_TARGET);

            command_lists[i]->ResourceBarrier(1, &barrier);

            ThrowIfFailed(
                command_lists[i]->Reset(command_allocator[i].Get(), nullptr));
        }
    };
    void Close(int begin, int end) {
        for (int i = begin; i < end; i++) {
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
                dx12::GpuCore::Instance()
                    .render_targets[dx12::GpuCore::Instance().frame_index]
                    .Get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT);
            command_lists[i]->ResourceBarrier(1, &barrier);
            ThrowIfFailed(command_lists[i]->Close());
        }

        dx12::GpuCore::Instance().command_queue->ExecuteCommandLists(
            NUM_COMAAND_LIST, CommandListCast(command_lists[0].GetAddressOf()));
    }

    ComPtr<ID3D12GraphicsCommandList> Get(int index) {
        return command_lists[index];
    }

    void OpenResource(int index) 
    { 
        command_allocator[index]->Reset();
        command_lists[index]->Reset(command_allocator[index].Get(), nullptr);
    }
    void CloseResource(int index) 
    { 
        command_lists[index]->Close();
        dx12::GpuCore::Instance().command_queue->ExecuteCommandLists(
			1, CommandListCast(command_lists[index].GetAddressOf()));

        dx12::GpuCore::Instance().command_queue->Signal(
			dx12::GpuCore::Instance().fence.Get(),
			dx12::GpuCore::Instance().fence_value);
    }

  private:
    ComPtr<ID3D12CommandAllocator> command_allocator[NUM_COMAAND_LIST];
    ComPtr<ID3D12GraphicsCommandList> command_lists[NUM_COMAAND_LIST];
};
} // namespace dx12
#endif