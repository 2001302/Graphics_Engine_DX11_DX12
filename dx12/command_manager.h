#ifndef _COMMAND_MANAGER
#define _COMMAND_MANAGER

#include "command_list.h"
#include "command_queue.h"

namespace graphics {

class GpuCommand {
  public:
    GpuCommand(){};

    void Initialize(ID3D12Device *device) {
        graphics_queue = new CommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        compute_queue = new CommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
        copy_queue = new CommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);

        graphics_queue->Create(device);
        compute_queue->Create(device);
        copy_queue->Create(device);

        graphics_context = new GraphicsCommandContext();
        compute_context = new ComputeCommandContext();
        copy_context = new CopyCommandContext();

        CreateNewCommandList(device, D3D12_COMMAND_LIST_TYPE_DIRECT,
                             graphics_context->GetListAdress(),
                             graphics_context->GetAllocatorAdress());

        CreateNewCommandList(device, D3D12_COMMAND_LIST_TYPE_COMPUTE,
                             compute_context->GetListAdress(),
                             compute_context->GetAllocatorAdress());

        CreateNewCommandList(device, D3D12_COMMAND_LIST_TYPE_COPY,
                             copy_context->GetListAdress(),
                             copy_context->GetAllocatorAdress());
    };

    CommandQueue *GraphicsQueue(void) { return graphics_queue; }
    CommandQueue *ComputeQueue(void) { return compute_queue; }
    CommandQueue *CopyQueue(void) { return copy_queue; }

    GraphicsCommandContext *GraphicsList(void) { return graphics_context; }
    ComputeCommandContext *ComputeList(void) { return compute_context; }
    CopyCommandContext *CopyList(void) { return copy_context; }

    void CreateNewCommandList(ID3D12Device *device,
                              D3D12_COMMAND_LIST_TYPE type,
                              ID3D12GraphicsCommandList **list,
                              ID3D12CommandAllocator **allocator) {
        assert(type != D3D12_COMMAND_LIST_TYPE_BUNDLE,
               "Bundles are not yet supported");
        switch (type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            *allocator = graphics_queue->RequestAllocator(device);
            break;
        case D3D12_COMMAND_LIST_TYPE_BUNDLE:
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            *allocator = compute_queue->RequestAllocator(device);
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            *allocator = copy_queue->RequestAllocator(device);
            break;
        }

        device->CreateCommandList(1, type, *allocator, nullptr,
                                  IID_PPV_ARGS(list));
        (*list)->SetName(L"CommandList");
        (*list)->Close();
    };

    void IdleGPU(void) {
        graphics_queue->WaitForIdle();
        compute_queue->WaitForIdle();
        copy_queue->WaitForIdle();
    }

    void Begin(D3D12_COMMAND_LIST_TYPE type) {
        switch (type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            graphics_context->Reset();
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            compute_context->Reset();
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            copy_context->Reset();
            break;
        default:
            throw std::exception("Not supported command list type");
            break;
        }
    };

    void Finish(D3D12_COMMAND_LIST_TYPE type,
                bool wait_for_completion = false) {
        uint64_t fence_value = 0;

        switch (type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            fence_value =
                graphics_queue->ExecuteCommandList(graphics_context->GetList());
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            fence_value =
                compute_queue->ExecuteCommandList(compute_context->GetList());
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            fence_value =
                copy_queue->ExecuteCommandList(copy_context->GetList());
            break;
        default:
            throw std::exception("Not supported command list type");
            break;
        }

        if (wait_for_completion)
            GetQueue(type)->WaitForFence(fence_value);
    };

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
    }

    CommandQueue *graphics_queue;
    CommandQueue *compute_queue;
    CommandQueue *copy_queue;

    GraphicsCommandContext *graphics_context;
    ComputeCommandContext *compute_context;
    CopyCommandContext *copy_context;
};

} // namespace graphics
#endif
