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

        graphics_list = new GraphicsCommandList();
        compute_list = new ComputeCommandList();
        copy_list = new CopyCommandList();

        auto graphics_command_list = graphics_list->GetList();
        auto graphics_command_allocator = graphics_list->GetAllocator();
        CreateNewCommandList(device, D3D12_COMMAND_LIST_TYPE_DIRECT,
                             &graphics_command_list,
                             &graphics_command_allocator);

        auto compute_command_list = compute_list->GetList();
        auto compute_command_allocator = compute_list->GetAllocator();
        CreateNewCommandList(device, D3D12_COMMAND_LIST_TYPE_COMPUTE,
                             &compute_command_list, &compute_command_allocator);

        auto copy_command_list = copy_list->GetList();
        auto copy_command_allocator = copy_list->GetAllocator();
        CreateNewCommandList(device, D3D12_COMMAND_LIST_TYPE_COPY,
                             &copy_command_list, &copy_command_allocator);
    };

    CommandQueue *GraphicsQueue(void) { return graphics_queue; }
    CommandQueue *ComputeQueue(void) { return compute_queue; }
    CommandQueue *CopyQueue(void) { return copy_queue; }

    GraphicsCommandList *GraphicsList(void) { return graphics_list; }
    ComputeCommandList *ComputeList(void) { return compute_list; }
    CopyCommandList *CopyList(void) { return copy_list; }

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
    };

    void IdleGPU(void) {
        graphics_queue->WaitForIdle();
        compute_queue->WaitForIdle();
        copy_queue->WaitForIdle();
    }

    void Start(D3D12_COMMAND_LIST_TYPE type) {
        switch (type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            graphics_list->Reset();
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            compute_list->Reset();
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            copy_list->Reset();
            break;
        default:
            throw std::exception("Not supported command list type");
            break;
        }
    };

    void Finish(D3D12_COMMAND_LIST_TYPE type) {
        uint64_t fence_value = 0;

        switch (type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            fence_value =
                graphics_queue->ExecuteCommandList(graphics_list->GetList());
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            fence_value =
                compute_queue->ExecuteCommandList(compute_list->GetList());
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            fence_value = copy_queue->ExecuteCommandList(copy_list->GetList());
            break;
        default:
            throw std::exception("Not supported command list type");
            break;
        }

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

    GraphicsCommandList *graphics_list;
    ComputeCommandList *compute_list;
    CopyCommandList *copy_list;
};

} // namespace graphics
#endif
