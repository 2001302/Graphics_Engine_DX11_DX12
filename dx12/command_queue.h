#ifndef _COMMAND_QUEUE
#define _COMMAND_QUEUE

#include "descriptor_heap.h"
#include <d3d12.h>
#include <memory>
#include <mutex>
#include <vector>
#include <wrl/client.h>

namespace graphics {
class CommandQueue {
  public:
    CommandQueue(D3D12_COMMAND_LIST_TYPE type)
        : type_(type), command_queue_(nullptr), fence_(nullptr),
          next_fence_value_((uint64_t)type << 56 | 1),
          last_completed_fence_value_((uint64_t)type << 56),
          fence_event_handle_(0){};
    ~CommandQueue() { Shutdown(); };
    void Shutdown() {
        if (command_queue_ == nullptr)
            return;

        CloseHandle(fence_event_handle_);

        fence_->Release();
        fence_ = nullptr;

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

        ASSERT_FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                          IID_PPV_ARGS(&fence_)));
        fence_->SetName(L"Fence");
        fence_->Signal((uint64_t)type_ << 56);

        fence_event_handle_ = CreateEvent(nullptr, false, false, nullptr);
        assert(fence_event_handle_ != NULL);
    }

    uint64_t IncrementFence(void) {
        std::lock_guard<std::mutex> LockGuard(fence_mutex_);
        command_queue_->Signal(fence_, next_fence_value_);
        return next_fence_value_++;
    };
    bool IsFenceComplete(uint64_t fence_value) {
        // Avoid querying the fence value by
        // testing against the last one seen.
        // The max() is to protect against an unlikely race condition that could
        // cause the last completed fence value to regress.
        if (fence_value > last_completed_fence_value_)
            last_completed_fence_value_ = (std::max)(
                last_completed_fence_value_, fence_->GetCompletedValue());

        return fence_value <= last_completed_fence_value_;
    };
    //void StallForFence(uint64_t FenceValue) {
    //    CommandQueue &Producer = GpuCore::Instance().GetCommandMgr()->GetQueue(
    //        (D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
    //    command_queue_->Wait(Producer.fence_, FenceValue);
    //};
    void StallForProducer(CommandQueue &Producer) {
        assert(Producer.next_fence_value_ > 0);
        command_queue_->Wait(Producer.fence_, Producer.next_fence_value_ - 1);
    };
    void WaitForFence(uint64_t FenceValue) {
        if (IsFenceComplete(FenceValue))
            return;

        // TODO:  Think about how this might affect a multi-threaded situation.
        // Suppose thread A wants to wait for fence 100, then thread B comes
        // along and wants to wait for 99.  If the fence can only have one event
        // set on completion, then thread B has to wait for 100 before it knows
        // 99 is ready.  Maybe insert sequential events?
        {
            std::lock_guard<std::mutex> LockGuard(event_mutex_);

            fence_->SetEventOnCompletion(FenceValue, fence_event_handle_);
            WaitForSingleObject(fence_event_handle_, INFINITE);
            last_completed_fence_value_ = FenceValue;
        }
    };
    void WaitForIdle(void) { WaitForFence(IncrementFence()); }

    ID3D12CommandQueue *Get() { return command_queue_; }

    uint64_t GetNextFenceValue() { return next_fence_value_; }

    ID3D12CommandAllocator *RequestAllocator(ID3D12Device *device) {
        uint64_t CompletedFence = fence_->GetCompletedValue();

        ID3D12CommandAllocator *allocator = nullptr;
        device->CreateCommandAllocator(type_, IID_PPV_ARGS(&allocator));
        return allocator;
    };
  private:
    uint64_t ExecuteCommandList(ID3D12CommandList *list) {
        std::lock_guard<std::mutex> LockGuard(fence_mutex_);

        ((ID3D12GraphicsCommandList *)list)->Close();

        // Kickoff the command list
        command_queue_->ExecuteCommandLists(1, &list);

        // Signal the next fence value (with the GPU)
        command_queue_->Signal(fence_, next_fence_value_);

        // And increment the fence value.
        return next_fence_value_++;
    };

    ID3D12CommandQueue *command_queue_;
    const D3D12_COMMAND_LIST_TYPE type_;

    std::mutex fence_mutex_;
    std::mutex event_mutex_;

    // Lifetime of these objects is managed by the descriptor cache
    ID3D12Fence *fence_;
    uint64_t next_fence_value_;
    uint64_t last_completed_fence_value_;
    HANDLE fence_event_handle_;
};
} // namespace graphics
#endif
