#ifndef _GRAPHICS_CORE
#define _GRAPHICS_CORE

#include "back_buffer.h"
#include "color_buffer.h"
#include "command_manager.h"
#include "common/logger.h"
#include "depth_buffer.h"
#include "heap_manager.h"

namespace graphics {
struct GlobalGpuBuffer {
    ColorBuffer *hdr_buffer;
    DepthBuffer *dsv_buffer;
};

class GpuCore {
  public:
    static GpuCore &Instance() {
        static GpuCore instance;
        return instance;
    }
    bool Initialize();
    bool InitializePix();
    bool InitializeDevice();
    bool InitializeSwapchain();
    bool InitializeBuffer();
    bool InitializeHeap();
    bool InitializeCommand();
    bool AllocateBuffer();
    void Shutdown();

    ID3D12Device *GetDevice() { return device.Get(); }
    IDXGISwapChain1 *GetSwapChain() { return swap_chain.Get(); }
    BackBuffer *GetDisplay() { return &back_buffer; }
    GpuCommand *GetCommand() { return &command_manager; }

    GpuHeap GetHeap() { return heap_manager; }
    GlobalGpuBuffer GetBuffers() { return buffers; }

  private:
    GpuCore() : swap_chain(0), device(0), buffers(){};
    ComPtr<IDXGISwapChain1> swap_chain;
    ComPtr<ID3D12Device> device;

    BackBuffer back_buffer;
    GpuHeap heap_manager;
    GpuCommand command_manager;
    GlobalGpuBuffer buffers;
};

} // namespace graphics
#endif
