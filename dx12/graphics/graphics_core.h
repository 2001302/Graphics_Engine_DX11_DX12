#ifndef _GRAPHICS_CORE
#define _GRAPHICS_CORE

#include "back_buffer.h"
#include "color_buffer.h"
#include "command_list.h"
#include "command_queue.h"
#include "depth_buffer.h"
#include "descriptor_heap.h"

namespace graphics {
class GpuBufferManager {
  public:
    GpuBufferManager(){};
    BackBuffer* back_buffer;
    ColorBuffer* hdr_buffer;
    ColorBuffer* ldr_buffer;
    DepthBuffer* dsv_buffer;
};

class GpuHeapManager {
  public:
    GpuHeapManager(){};
    DescriptorHeap* view_heap;    // CBV_SRV_UAV
    DescriptorHeap* sampler_heap; // SAMPLER
    DescriptorHeap* rtv_heap;     // RTV
    DescriptorHeap* dsv_heap;     // DSV
};

class GpuCommandManager {
  public:
    GpuCommandManager(ID3D12Device* device)
        : graphics_queue(D3D12_COMMAND_LIST_TYPE_DIRECT),
          compute_queue(D3D12_COMMAND_LIST_TYPE_COMPUTE),
          copy_queue(D3D12_COMMAND_LIST_TYPE_COPY) {
        graphics_queue.Create(device);
        compute_queue.Create(device);
        copy_queue.Create(device);
    };

    CommandQueue &GetGraphicsQueue(void) { return graphics_queue; }
    CommandQueue &GetComputeQueue(void) { return compute_queue; }
    CommandQueue &GetCopyQueue(void) { return copy_queue; }

    CommandQueue &
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

    ID3D12CommandQueue *GetCommandQueue() {
        return graphics_queue.GetCommandQueue();
    }
    void CreateNewCommandList(ID3D12Device *device,
                              D3D12_COMMAND_LIST_TYPE type,
                              ID3D12GraphicsCommandList **list,
                              ID3D12CommandAllocator **allocator) {
        assert(type != D3D12_COMMAND_LIST_TYPE_BUNDLE,
               "Bundles are not yet supported");
        switch (type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            *allocator = graphics_queue.RequestAllocator(device);
            break;
        case D3D12_COMMAND_LIST_TYPE_BUNDLE:
            break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            *allocator = compute_queue.RequestAllocator(device);
            break;
        case D3D12_COMMAND_LIST_TYPE_COPY:
            *allocator = copy_queue.RequestAllocator(device);
            break;
        }

        device->CreateCommandList(1, type, *allocator, nullptr,
                                  IID_PPV_ARGS(list));
        (*list)->SetName(L"CommandList");
    };

    void IdleGPU(void) {
        graphics_queue.WaitForIdle();
        compute_queue.WaitForIdle();
        copy_queue.WaitForIdle();
    }

  private:
    CommandQueue graphics_queue;
    CommandQueue compute_queue;
    CommandQueue copy_queue;
};

class GpuContextManager {
  public:
    ID3D12CommandAllocator *graphics_list_allocator;
    ID3D12CommandAllocator *compute_list_allocator;
    ID3D12CommandAllocator *copy_list_allocator;
    ID3D12GraphicsCommandList *graphics_list;
    ID3D12GraphicsCommandList *compute_list;
    ID3D12GraphicsCommandList *copy_list;
};

class GpuCore {
  public:
    static GpuCore &Instance() {
        static GpuCore instance;
        return instance;
    }
    bool Initialize();
    ID3D12Device *GetDevice() { return device.Get(); }
    GpuHeapManager *GetHeapMgr() { return heap_manager; }
    GpuBufferManager *GetBufferMgr() { return buffer_manager; }
    GpuCommandManager *GetCommandMgr() { return command_manager; }
    GpuContextManager *GetContextMgr() { return context_manager; }

  private:
    GpuCore() : swap_chain(0), device(0) {}
    ComPtr<IDXGISwapChain1> swap_chain;
    ComPtr<ID3D12Device> device;
    GpuHeapManager* heap_manager;
    GpuBufferManager* buffer_manager;
    GpuCommandManager* command_manager;
    GpuContextManager* context_manager;
};

} // namespace graphics
#endif
