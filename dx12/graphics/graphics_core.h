#ifndef _GRAPHICS_CORE
#define _GRAPHICS_CORE

#include "graphics_device.h"
#include "descriptor_heap.h"
#include "command_pool.h"
#include "color_buffer.h"
#include "depth_buffer.h"
#include "back_buffer.h"

namespace graphics {
class GpuBufferManager {
    std::shared_ptr<BackBuffer> back_buffer;
    std::shared_ptr<ColorBuffer> hdr_buffer;
    std::shared_ptr<ColorBuffer> ldr_buffer;
    std::shared_ptr<DepthBuffer> dsv_buffer;
};
class GpuHeapManager {
    std::shared_ptr<DescriptorHeap> view_heap;    // CBV_SRV_UAV
    std::shared_ptr<DescriptorHeap> sampler_heap; // SAMPLER
    std::shared_ptr<DescriptorHeap> rtv_heap;     // RTV
    std::shared_ptr<DescriptorHeap> dsv_heap;     // DSV
};
class GpuCommandManager {
    //Gpu command queue 를 관리 
    std::shared_ptr<CommandPool> command_pool;
};
class GpuContextManager {
    //Gpu command list 를 관리
};
class GpuCore {
  public:
    static GpuCore &Instance() {
        static GpuCore instance;
        return instance;
    }
    bool Initialize();
    ID3D12Device* GetDevice() { return device.Get(); }
    GpuBufferManager *GetBufferMgr() { return buffer_manager.get(); }
    GpuCommandManager *GetCommandMgr() { return command_manager.get(); }
    GpuContextManager *GetContextMgr() { return context_manager.get(); }

  private:
    GpuCore() : swap_chain(0), device(0) {}
    ComPtr<IDXGISwapChain3> swap_chain;
    ComPtr<ID3D12Device> device;
    std::shared_ptr<GpuBufferManager> buffer_manager;
    std::shared_ptr<GpuCommandManager> command_manager;
    std::shared_ptr<GpuContextManager> context_manager;
};

} // namespace graphics
#endif
