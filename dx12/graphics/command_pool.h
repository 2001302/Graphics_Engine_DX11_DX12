#ifndef _COMMAND_POOL
#define _COMMAND_POOL

#include "graphics_core.h"

namespace dx12 {
class CommandPool {
  public:
    CommandPool(){};
    void Initialize() {
        for (int i = 0; i < 2; i++) {
            ThrowIfFailed(GpuCore::Instance().device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&command_allocator[i])));
            ThrowIfFailed(GpuCore::Instance().device->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator[i].Get(),
                nullptr, IID_PPV_ARGS(&command_lists[i])));
            command_lists[i]->Close();
        }
    }
    void Open() {
        for (int i = 0; i < 2; i++) {
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
    void Close() {
        for (int i = 0; i < 2; i++) {
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
            2, CommandListCast(command_lists[0].GetAddressOf()));
    }
    ID3D12GraphicsCommandList *Get(int index) {
        return command_lists[index].Get();
    }

  private:
    ComPtr<ID3D12CommandAllocator> command_allocator[2];
    ComPtr<ID3D12GraphicsCommandList> command_lists[2];
};
} // namespace dx12
#endif