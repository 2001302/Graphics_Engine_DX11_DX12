#ifndef _GRAPHICSMANAGER
#define _GRAPHICSMANAGER

#define MAX_LIGHTS 3
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

#include <d3d12.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <dxgi.h>    // DXGIFactory
#include <dxgi1_4.h> // DXGIFactory4
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h> // ComPtr

#include "../foundation/env.h"
#include "d3dx12.h"

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using Microsoft::WRL::ComPtr;

namespace dx12 {
inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        throw std::exception();
    }
}

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

    ComPtr<ID3D12Resource> renderTargets[2];
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    UINT rtvDescriptorSize;

    ComPtr<ID3D12DescriptorHeap> srvHeap;
    ComPtr<ID3D12DescriptorHeap> cbvHeap; //단위는 어떻게 되는지 확인 필요 (global,node,object)

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
