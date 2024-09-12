#ifndef _HEAP_MANAGER
#define _HEAP_MANAGER

#include "descriptor_heap.h"

namespace graphics {

class GpuHeap {
  public:
    GpuHeap()
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
    DescriptorHeap *GetViewHeap() { return heap_view; }
    DescriptorHeap *GetSamplerHeap() { return heap_sampler; }
    DescriptorHeap *GetRTVHeap() { return heap_RTV; }
    DescriptorHeap *GetDSVHeap() { return heap_DSV; }

  private:
    DescriptorHeap *heap_view;    // CBV_SRV_UAV
    DescriptorHeap *heap_sampler; // SAMPLER
    DescriptorHeap *heap_RTV;     // RTV
    DescriptorHeap *heap_DSV;     // DSV
};

} // namespace graphics
#endif
