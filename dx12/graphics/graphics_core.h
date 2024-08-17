#ifndef _GRAPHICSMANAGER
#define _GRAPHICSMANAGER

#define MAX_LIGHTS 3
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

#include "../foundation/env.h"
#include "graphics_common.h"

namespace dx12 {
class GpuCore {
  public:
    static GpuCore &Instance() {
        static GpuCore instance;
        return instance;
    }
    bool Initialize();

    bool useMSAA = true;
    UINT num_quality_levels = 0;

    ComPtr<IDXGISwapChain3> swap_chain;
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12PipelineState> pipelineState;
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12CommandQueue> command_queue;
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ComPtr<ID3D12RootSignature> rootSignature;

    ComPtr<ID3D12Resource> renderTargets[2];
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> srvHeap;
    UINT rtvDescriptorSize;

    ComPtr<ID3D12DescriptorHeap> cbvHeap;

    // Synchronization objects.
    UINT frameIndex;
    HANDLE fenceEvent;
    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue;

    D3D12_VIEWPORT viewport;

  private:
    GpuCore()
        : swap_chain(0), device(0), command_queue(0),
          viewport(D3D12_VIEWPORT()) {}
};

} // namespace dx12
#endif
