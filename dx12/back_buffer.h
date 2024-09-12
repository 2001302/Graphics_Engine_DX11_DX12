#ifndef _BACK_BUFFER
#define _BACK_BUFFER
#define SWAP_CHAIN_BUFFER_COUNT 2

#include "gpu_resource.h"

namespace graphics {
class BackBuffer {
  public:
    BackBuffer()
        : device_(0), index_(), rtv_handle_(), rtv_heap_(0), current_index_(0),
          current_state_(){};
    void Create(ID3D12Device *device, DescriptorHeap *heap,
                IDXGISwapChain1 *swap_chain) {
        device_ = device;
        rtv_heap_ = heap;
        for (UINT n = 0; n < SWAP_CHAIN_BUFFER_COUNT; n++) {
            ASSERT_FAILED(
                swap_chain->GetBuffer(n, IID_PPV_ARGS(&resource_[n])));
            current_state_[n] = D3D12_RESOURCE_STATE_COMMON;
        }
    };
    void Allocate() {
        for (UINT n = 0; n < SWAP_CHAIN_BUFFER_COUNT; n++) {
            rtv_heap_->AllocateDescriptor(rtv_handle_[n], index_[n]);
            device_->CreateRenderTargetView(resource_[n].Get(), nullptr,
                                            rtv_handle_[n]);
        }
    };
    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() {
        return rtv_handle_[current_index_];
    };
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() {
        return rtv_heap_->GetGpuHandle(current_index_);
    };
    ID3D12Resource *Get() { return resource_[current_index_].Get(); };
    D3D12_RESOURCE_STATES GetCurrentState() {
        return current_state_[current_index_];
    };
    void SetCurrentState(D3D12_RESOURCE_STATES state) {
        current_state_[current_index_] = state;
    };

  private:
    ID3D12Device *device_;
    DescriptorHeap *rtv_heap_;
    ComPtr<ID3D12Resource> resource_[SWAP_CHAIN_BUFFER_COUNT];
    UINT index_[SWAP_CHAIN_BUFFER_COUNT];
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle_[SWAP_CHAIN_BUFFER_COUNT];
    D3D12_RESOURCE_STATES current_state_[SWAP_CHAIN_BUFFER_COUNT];
    UINT current_index_;
};
} // namespace graphics
#endif