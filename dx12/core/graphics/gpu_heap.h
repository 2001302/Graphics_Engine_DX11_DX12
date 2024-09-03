#ifndef _DISCRIPTOR_HEAP_STACK
#define _DISCRIPTOR_HEAP_STACK

#include "graphics_core.h"
#include "graphics_pso.h"

namespace dx12 {
class GpuHeap {
  public:
    GpuHeap(UINT num_descriptors, D3D12_DESCRIPTOR_HEAP_TYPE type,
            UINT NodeMask);
    ID3D12DescriptorHeap &GetDescriptorHeap();
    ID3D12DescriptorHeap &GetSamplerHeap();
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT descriptor_index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetSamplerGpuHandle();
    void AllocateDescriptor(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &cpu_handle,
                            _Out_ UINT &descriptor_heap_index);

  private:
    ComPtr<ID3D12DescriptorHeap> descriptor_heap;
    UINT descriptors_allocated = 0;
    UINT descriptor_size;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor_heap_cpu_handle;

    ComPtr<ID3D12DescriptorHeap> sampler_heap;
    UINT descriptor_size_sampler;
    D3D12_CPU_DESCRIPTOR_HANDLE sampler_heap_cpu_handle;
};

} // namespace dx12
#endif