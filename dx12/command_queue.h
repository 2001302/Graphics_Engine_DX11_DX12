#ifndef _COMMAND_QUEUE
#define _COMMAND_QUEUE

#include "command_list.h"
#include "dynamic_descriptor_heap.h"
#include <d3d12.h>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include <wrl/client.h>

namespace graphics {
class CommandQueue {

    friend class GpuCommand;

  public:
    CommandQueue(D3D12_COMMAND_LIST_TYPE type)
        : type_(type), command_queue_(nullptr){};
    ~CommandQueue(){};

    void Shutdown() {
        if (command_queue_ == nullptr)
            return;

        command_queue_->Release();
        command_queue_ = nullptr;
    };

    void Create(ID3D12Device *device) {
        assert(device != nullptr);

        D3D12_COMMAND_QUEUE_DESC queue_desc = {};
        queue_desc.Type = type_;
        queue_desc.NodeMask = 1;
        device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue_));
        command_queue_->SetName(L"CommandQueue");
    }

    ID3D12CommandQueue *Get() { return command_queue_; }

    ID3D12CommandAllocator *RequestAllocator(ID3D12Device *device) {
        ID3D12CommandAllocator *allocator = nullptr;
        device->CreateCommandAllocator(type_, IID_PPV_ARGS(&allocator));
        return allocator;
    };

  private:
    void ExecuteCommandList(CommandContext *context) {

        context->Close();
        context->ExecuteCommandLists(this->command_queue_);
    };

    ID3D12CommandQueue *command_queue_;
    const D3D12_COMMAND_LIST_TYPE type_;
};
} // namespace graphics
#endif
