#ifndef _GRAPHICS_CORE
#define _GRAPHICS_CORE

#include "back_buffer.h"
#include "color_buffer.h"
#include "command_list.h"
#include "command_queue.h"
#include "depth_buffer.h"
#include "descriptor_heap.h"

namespace graphics {
struct GlobalGpuBuffer {
    BackBuffer back_buffer;
    ColorBuffer hdr_buffer;
    ColorBuffer ldr_buffer;
    DepthBuffer dsv_buffer;
};

class GpuHeapManager {
  public:
    GpuHeapManager()
        : heap_view(0), heap_sampler(0), heap_RTV(0), heap_DSV(0){};
    void Initialize(ID3D12Device *device, UINT num_descriptor) {
        heap_RTV = new DescriptorHeap(device, num_descriptor,
                                      D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
        heap_view = new DescriptorHeap(
            device, num_descriptor, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
        heap_DSV = new DescriptorHeap(device, num_descriptor,
                                      D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
        heap_sampler = new DescriptorHeap(
            device, num_descriptor, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1);
    };
    DescriptorHeap &GetViewHeap() { return *heap_view; }
    DescriptorHeap &GetSamplerHeap() { return *heap_sampler; }
    DescriptorHeap &GetRTVHeap() { return *heap_RTV; }
    DescriptorHeap &GetDSVHeap() { return *heap_DSV; }

  private:
    DescriptorHeap *heap_view;    // CBV_SRV_UAV
    DescriptorHeap *heap_sampler; // SAMPLER
    DescriptorHeap *heap_RTV;     // RTV
    DescriptorHeap *heap_DSV;     // DSV
};

class GpuCommandManager {
  public:
    GpuCommandManager(ID3D12Device *device)
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

struct GpuContextManager {
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
    bool InitializeBuffer() {
        buffer_manager.back_buffer.Create(
            device.Get(), &heap_manager->GetRTVHeap(), swap_chain.Get());
        buffer_manager.hdr_buffer.Create(
            device.Get(), &heap_manager->GetRTVHeap(),
            &heap_manager->GetViewHeap(), DXGI_FORMAT_R16G16B16A16_FLOAT);
        buffer_manager.ldr_buffer.Create(
            device.Get(), &heap_manager->GetRTVHeap(),
            &heap_manager->GetViewHeap(), DXGI_FORMAT_R8G8B8A8_UNORM);
        buffer_manager.dsv_buffer.Create(
            device.Get(), &heap_manager->GetDSVHeap(), DXGI_FORMAT_D32_FLOAT);

        buffer_manager.back_buffer.Allocate();
        buffer_manager.hdr_buffer.Allocate();
        buffer_manager.ldr_buffer.Allocate();
        buffer_manager.dsv_buffer.Allocate();
        return true;
    };
    GlobalGpuBuffer Buffer() { return buffer_manager; }

    ID3D12Device *GetDevice() { return device.Get(); }
    GpuHeapManager *GetHeapMgr() { return heap_manager.get(); }
    GpuCommandManager *GetCommandMgr() { return command_manager.get(); }
    GpuContextManager *GetContextMgr() { return context_manager.get(); }

  private:
    GpuCore()
        : swap_chain(0), device(0), heap_manager(0), command_manager(0),
          context_manager(0) {}
    ComPtr<IDXGISwapChain1> swap_chain;
    ComPtr<ID3D12Device> device;
    std::shared_ptr<GpuHeapManager> heap_manager;
    std::shared_ptr<GpuCommandManager> command_manager;
    std::shared_ptr<GpuContextManager> context_manager;
    GlobalGpuBuffer buffer_manager;
};

} // namespace graphics
#endif
