#ifndef _GRAPHICS_CORE
#define _GRAPHICS_CORE

#include "command_manager.h"
#include "heap_manager.h"
#include "../../pch.h"
#include <util/logger.h>
#include <structure/env.h>

namespace graphics {
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
    bool InitializeHeap();
    bool InitializeCommand();
    void Shutdown();

    ID3D12Device *GetDevice() { return device.Get(); }
    IDXGISwapChain1 *GetSwapChain() { return swap_chain.Get(); }
    GpuCommand *GetCommand() { return &command_manager; }

    GpuHeap GetHeap() { return heap_manager; }

  private:
    GpuCore() : swap_chain(0), device(0){};
    ComPtr<IDXGISwapChain1> swap_chain;
    ComPtr<ID3D12Device> device;

    QueryHeap query_heap;
    GpuHeap heap_manager;
    GpuCommand command_manager;
};

} // namespace graphics
#endif
