#include "command_manager.h"

namespace graphics {
void GpuCommand::Initialize(ID3D12Device *device) {
    device_ = device;

    graphics_queue = new CommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    compute_queue = new CommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
    copy_queue = new CommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);

    graphics_queue->Create(device_);
    compute_queue->Create(device_);
    copy_queue->Create(device_);
};

CommandQueue *GpuCommand::Queue(D3D12_COMMAND_LIST_TYPE type) {
    switch (type) {
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return compute_queue;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        return copy_queue;
    default:
        return graphics_queue;
    }
};

CommandContext *GpuCommand::AllocateContext(ID3D12Device *device,
                                            D3D12_COMMAND_LIST_TYPE type) {
    std::lock_guard<std::mutex> lock_guard(context_allocation_mutex_);

    CommandContext *context = nullptr;
    if (available_contexts_[type].empty()) {

        if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
            context = new ComputeCommandContext();
        else if (type == D3D12_COMMAND_LIST_TYPE_COPY)
            context = new CopyCommandContext();
        else
            context = new GraphicsCommandContext();

        context->Initialize(device);
        context_pool_[type].push(context);

        common::Logger::Debug(
            "Allocated new command context. Retired context #" +
            retired_contexts_->size());
    } else {
        context = available_contexts_[type].front();
        available_contexts_[type].pop();
        context->Reset();
        context_pool_[type].push(context);
    }

    assert(context != nullptr);

    return context;
}

} // namespace graphics