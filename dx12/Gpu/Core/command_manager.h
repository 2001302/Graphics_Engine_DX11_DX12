#ifndef _COMMAND_MANAGER
#define _COMMAND_MANAGER

#include "../command/command_list.h"
#include "../command/command_queue.h"
#include <util/logger.h>
#include <type_traits>

namespace graphics {

class GpuCommand {
  public:
    GpuCommand()
        : device_(nullptr), query_heap_(nullptr), graphics_queue(nullptr),
          compute_queue(nullptr), copy_queue(nullptr),
          context_allocation_mutex_(){};

    void Initialize(ID3D12Device *device, ID3D12QueryHeap *query_heap);
    CommandQueue *Queue(D3D12_COMMAND_LIST_TYPE type);

    template <typename T> T *Begin(const wchar_t *label) {

        D3D12_COMMAND_LIST_TYPE type;
        if (std::is_same<T, GraphicsCommandContext>::value) {
            type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        } else if (std::is_same<T, ComputeCommandContext>::value) {
            type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        } else if (std::is_same<T, CopyCommandContext>::value) {
            type = D3D12_COMMAND_LIST_TYPE_COPY;
        } else
            throw std::exception("Invalid command list type");

        T *new_context = (T *)AllocateContext(device_, type);

        if (type == D3D12_COMMAND_LIST_TYPE_DIRECT ||
            type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
            new_context->StartTiming(query_heap_, label);

        return new_context;
    };

    void Finish(CommandContext *context, bool wait_for_completion = false) {

        if (context->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT ||
            context->GetType() == D3D12_COMMAND_LIST_TYPE_COMPUTE)
            context->EndTiming(query_heap_);

        context->Close();
        context->ExecuteCommandLists(Queue(context->GetType())->Get());

        retired_contexts_[context->GetType()].push(context);
        context_pool_[context->GetType()].pop();

        if (wait_for_completion)
            Wait(context->GetType());
    };

    template <typename T> T *Context() {

        D3D12_COMMAND_LIST_TYPE type;
        if (std::is_same<T, GraphicsCommandContext>::value) {
            type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        } else if (std::is_same<T, ComputeCommandContext>::value) {
            type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        } else if (std::is_same<T, CopyCommandContext>::value) {
            type = D3D12_COMMAND_LIST_TYPE_COPY;
        } else
            throw std::exception("Invalid command list type");

        auto &context_pool = context_pool_[type];

        if (!context_pool.empty()) {
            return (T *)context_pool.front();
        } else
            throw std::exception("No available command list in the pool");
    };

    void Wait(D3D12_COMMAND_LIST_TYPE type) {
        common::ScopeStopWatch stop_watch("Waiting for completion");

        while (!retired_contexts_[type].empty()) {

            retired_contexts_[type].front()->IsFenceComplete(
                Queue(type)->Get());

            auto context = retired_contexts_[type].front();
            available_contexts_[type].push(context);
            retired_contexts_[type].pop();
        }

    }

  private:
    CommandContext *AllocateContext(ID3D12Device *device,
                                    D3D12_COMMAND_LIST_TYPE type);

    ID3D12Device *device_;
    ID3D12QueryHeap *query_heap_;

    CommandQueue *graphics_queue;
    CommandQueue *compute_queue;
    CommandQueue *copy_queue;

    std::queue<CommandContext *> available_contexts_[4];
    std::queue<CommandContext *> retired_contexts_[4];
    std::queue<CommandContext *> context_pool_[4];
    std::mutex context_allocation_mutex_;
};

} // namespace graphics
#endif
