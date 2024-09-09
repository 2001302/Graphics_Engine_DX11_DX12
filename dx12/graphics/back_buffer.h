#ifndef _BACK_BUFFER
#define _BACK_BUFFER
#define SWAP_CHAIN_BUFFER_COUNT 3

#include "gpu_resource.h"

namespace graphics {
class BackBuffer {
  public:
    BackBuffer() : index_(), rtv_handle_(), rtv_heap_(0){};
    BackBuffer(IDXGISwapChain1 *swap_chain) : BackBuffer() {
        for (UINT n = 0; n < SWAP_CHAIN_BUFFER_COUNT; n++) {
            ThrowIfFailed(
                swap_chain->GetBuffer(n, IID_PPV_ARGS(&resource_[n])));
        }
    };
    void Allocate(ID3D12Device *device, DescriptorHeap *heap) {
        for (UINT n = 0; n < SWAP_CHAIN_BUFFER_COUNT; n++) {
            heap->AllocateDescriptor(rtv_handle_[n], index_[n]);
            device->CreateRenderTargetView(resource_[n].Get(), nullptr,
                                           rtv_handle_[n]);
        }
        rtv_heap_ = heap;
    };
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT index) {
        return rtv_heap_->GetGpuHandle(index);
    };

  private:
    DescriptorHeap *rtv_heap_;
    ComPtr<ID3D12Resource> resource_[SWAP_CHAIN_BUFFER_COUNT];
    UINT index_[SWAP_CHAIN_BUFFER_COUNT];
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle_[SWAP_CHAIN_BUFFER_COUNT];
};
} // namespace graphics
#endif