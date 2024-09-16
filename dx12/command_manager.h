#ifndef _COMMAND_MANAGER
#define _COMMAND_MANAGER

#include "command_list.h"
#include "command_queue.h"

namespace graphics {

class GpuCommand {
  public:
    GpuCommand()
        : device_(nullptr), graphics_queue(nullptr), compute_queue(nullptr),
          copy_queue(nullptr), context_allocation_mutex_(){};

    void Initialize(ID3D12Device *device) {
        device_ = device;

        graphics_queue = new CommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        compute_queue = new CommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
        copy_queue = new CommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);

        graphics_queue->Create(device_);
        compute_queue->Create(device_);
        copy_queue->Create(device_);
    };

    CommandContext *Begin(D3D12_COMMAND_LIST_TYPE type, const wchar_t *label) {
        CommandContext *new_context =
            AllocateContext(device_, D3D12_COMMAND_LIST_TYPE_DIRECT);
        new_context->PIXBeginEvent(label);
        return new_context;
    };

    void Finish(CommandContext *context, bool wait_for_completion = false) {

        context->PIXEndEvent();
        uint64_t fence_value = 0;

        fence_value = GetQueue(context->GetType())
                          ->ExecuteCommandList(context->GetList());

        if (wait_for_completion)
            GetQueue(context->GetType())->WaitForFence(fence_value);

        available_contexts_[context->GetType()].push(context);
        context_pool_[context->GetType()].pop();
    };

    CommandContext *Rent(D3D12_COMMAND_LIST_TYPE type) {
        CommandContext *new_context = nullptr;

        auto &context_pool = context_pool_[type];

        if (!context_pool.empty()) {
            new_context = context_pool.front();
        } else
            throw std::exception("No available command list in the pool");

        return new_context;
    };

    CommandQueue *GraphicsQueue(void) { return graphics_queue; }

  private:
    CommandQueue *
    GetQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT) {
        switch (Type) {
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            return compute_queue;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            return copy_queue;
        default:
            return graphics_queue;
        }
    };

    CommandContext *AllocateContext(ID3D12Device *device,
                                    D3D12_COMMAND_LIST_TYPE type) {
        std::lock_guard<std::mutex> lock_guard(context_allocation_mutex_);

        auto &available_contexts = available_contexts_[type];

        CommandContext *context = nullptr;
        if (available_contexts.empty()) {
            context = new CommandContext();
            context->Initialize(device, type);
            context_pool_[type].push(context);
        } else {
            context = available_contexts.front();
            available_contexts.pop();
            context->Reset();
            context_pool_[type].push(context);
        }

        assert(context != nullptr);

        return context;
    }

    ID3D12Device *device_;

    CommandQueue *graphics_queue;
    CommandQueue *compute_queue;
    CommandQueue *copy_queue;

    std::queue<CommandContext *> available_contexts_[4];
    std::queue<CommandContext *> context_pool_[4];
    std::mutex context_allocation_mutex_;
};

} // namespace graphics
#endif
