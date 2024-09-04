#ifndef _COMMAND_POOL
#define _COMMAND_POOL

#define NUM_COMAAND_LIST 1

#include "graphics_core.h"

namespace graphics {
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
    void Open() {
        for (int i = 0; i < NUM_COMAAND_LIST; i++) {
            command_allocator[i]->Reset();
            ThrowIfFailed(
                command_lists[i]->Reset(command_allocator[i].Get(), nullptr));
        }
    };
    void Close() {
        for (int i = 0; i < NUM_COMAAND_LIST; i++) {
            ThrowIfFailed(command_lists[i]->Close());
        }

        GpuCore::Instance().command_queue->ExecuteCommandLists(
            NUM_COMAAND_LIST, CommandListCast(command_lists[0].GetAddressOf()));
    }

    ComPtr<ID3D12GraphicsCommandList> Get(int index) {
        return command_lists[index];
    }

  private:
    ComPtr<ID3D12CommandAllocator> command_allocator[NUM_COMAAND_LIST];
    ComPtr<ID3D12GraphicsCommandList> command_lists[NUM_COMAAND_LIST];
};
} // namespace dx12
#endif