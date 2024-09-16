#ifndef _COMMAND_MANAGER
#define _COMMAND_MANAGER

#include "command_list.h"
#include "command_queue.h"
#include <type_traits>

namespace graphics {

class GpuCommand {
  public:
    GpuCommand()
        : device_(nullptr), graphics_queue(nullptr), compute_queue(nullptr),
          copy_queue(nullptr), context_allocation_mutex_(){};

    void Initialize(ID3D12Device *device);
    void Finish(CommandContext *context, bool wait_for_completion = false);

    CommandQueue *
    Queue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);

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
        new_context->PIXBeginEvent(label);
        return new_context;
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

  private:
    CommandContext *AllocateContext(ID3D12Device *device,
                                    D3D12_COMMAND_LIST_TYPE type);

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
