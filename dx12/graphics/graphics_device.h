#ifndef _GRAPHICSDEVICE
#define _GRAPHICSDEVICE

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
#include <stdint.h>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h> // ComPtr

#include "foundation/env.h"
#include "d3dx12.h"

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using Microsoft::WRL::ComPtr;

namespace graphics {
inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        throw std::exception();
    }
}

class GpuDevice {
  public:
    static GpuDevice &Get() {
        static GpuDevice instance;
        return instance;
    }
    bool InitializeGPU();
    void CreateBuffer();
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetHandleHDR();
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetHandleLDR();
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetHandleFLIP();
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetHandleDSV();

    bool useMSAA = true;

    ComPtr<IDXGISwapChain3> swap_chain;
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> command_queue;

    ComPtr<ID3D12DescriptorHeap> heap_HDR;
    ComPtr<ID3D12Resource> resource_HDR;

    ComPtr<ID3D12DescriptorHeap> heap_LDR;
    ComPtr<ID3D12Resource> resource_LDR;

    ComPtr<ID3D12DescriptorHeap> heap_FLIP;
    ComPtr<ID3D12Resource> resource_FLIP[2];

    ComPtr<ID3D12DescriptorHeap> heap_DSV;
    ComPtr<ID3D12Resource> resourcce_DSV;

    D3D12_VIEWPORT viewport;
    CD3DX12_RECT scissorRect;

    UINT frame_index;
    HANDLE fence_event;
    ComPtr<ID3D12Fence> fence;
    UINT64 fence_value;

  private:
    GpuDevice()
        : swap_chain(0), device(0), viewport(D3D12_VIEWPORT()), frame_index(0),
          fence_value(0), fence_event(0) {}
};

} // namespace dx12
#endif
