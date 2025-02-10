#ifndef _BACK_BUFFER
#define _BACK_BUFFER
#define SWAP_CHAIN_BUFFER_COUNT 3

#include "gpu_resource.h"

namespace graphics {
class BackBuffer {
  public:
    BackBuffer()
        : index_(), rtv_handle_(), rtv_heap_(0), current_index_(0),
          current_state_(){};
    static BackBuffer *Create(ID3D12Device *device, DynamicDescriptorHeap *heap,
                              IDXGISwapChain1 *swap_chain) {
        auto back_buffer = new BackBuffer();
        back_buffer->Initialize(device, heap, swap_chain);
        return back_buffer;
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
    void MoveToNext() {
        current_index_ = (current_index_ + 1) % SWAP_CHAIN_BUFFER_COUNT;
    };

  private:
    void Initialize(ID3D12Device *device, DynamicDescriptorHeap *heap,
                IDXGISwapChain1 *swap_chain) {
        rtv_heap_ = heap;

        for (UINT n = 0; n < SWAP_CHAIN_BUFFER_COUNT; n++) {
            ASSERT_FAILED(
                swap_chain->GetBuffer(n, IID_PPV_ARGS(&resource_[n])));
            current_state_[n] = D3D12_RESOURCE_STATE_COMMON;
        }

        for (UINT n = 0; n < SWAP_CHAIN_BUFFER_COUNT; n++) {
            rtv_heap_->AllocateDescriptor(rtv_handle_[n], index_[n]);
            device->CreateRenderTargetView(resource_[n].Get(), nullptr,
                                           rtv_handle_[n]);
        }
    };
    DynamicDescriptorHeap *rtv_heap_;
    ComPtr<ID3D12Resource> resource_[SWAP_CHAIN_BUFFER_COUNT];
    UINT index_[SWAP_CHAIN_BUFFER_COUNT];
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle_[SWAP_CHAIN_BUFFER_COUNT];
    D3D12_RESOURCE_STATES current_state_[SWAP_CHAIN_BUFFER_COUNT];
    UINT current_index_;
};
} // namespace graphics
#endif