#ifndef _GPU_HEAP
#define _GPU_HEAP

#include <d3d12.h>
#include <wrl/client.h>
#include "graphics_pso.h"
#include "graphics_common.h"

namespace graphics {
class DescriptorHeap {
  public:
    DescriptorHeap(ID3D12Device* device, UINT num_descriptors,
                   D3D12_DESCRIPTOR_HEAP_TYPE type,
            UINT NodeMask);
    ID3D12DescriptorHeap &Get();
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT descriptor_index);
    void AllocateDescriptor(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &cpu_handle,
                            _Out_ UINT &descriptor_heap_index);

  private:
    ComPtr<ID3D12DescriptorHeap> descriptor_heap;
    UINT descriptors_allocated = 0;
    UINT descriptor_size;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor_heap_cpu_handle;
};

} // namespace dx12
#endif