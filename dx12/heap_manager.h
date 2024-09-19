#ifndef _HEAP_MANAGER
#define _HEAP_MANAGER

#include "dynamic_descriptor_heap.h"
#include "query_heap.h"

namespace graphics {

class GpuHeap {
  public:
    GpuHeap()
        : heap_view(0), heap_sampler(0), heap_RTV(0), heap_DSV(0){};
    void Initialize(ID3D12Device *device, UINT num_descriptor) {
        heap_RTV = new DynamicDescriptorHeap(device, num_descriptor,
                                             D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1);
        heap_view = new DynamicDescriptorHeap(
            device, num_descriptor, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
        heap_DSV = new DynamicDescriptorHeap(device, num_descriptor,
                                             D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
        heap_sampler = new DynamicDescriptorHeap(
            device, num_descriptor, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 1);
    };
    void Shutdown() {
        if (heap_view) {
            delete heap_view;
            heap_view = 0;
        }
        if (heap_sampler) {
            delete heap_sampler;
            heap_sampler = 0;
        }
        if (heap_RTV) {
            delete heap_RTV;
            heap_RTV = 0;
        }
        if (heap_DSV) {
            delete heap_DSV;
            heap_DSV = 0;
        }
    };

    DynamicDescriptorHeap *View() { return heap_view; }
    DynamicDescriptorHeap *Sampler() { return heap_sampler; }
    DynamicDescriptorHeap *RTV() { return heap_RTV; }
    DynamicDescriptorHeap *DSV() { return heap_DSV; }

  private:
    DynamicDescriptorHeap *heap_view;    // CBV_SRV_UAV
    DynamicDescriptorHeap *heap_sampler; // SAMPLER
    DynamicDescriptorHeap *heap_RTV;     // RTV
    DynamicDescriptorHeap *heap_DSV;     // DSV
};

} // namespace graphics
#endif
