#ifndef _GPU_HEAP
#define _GPU_HEAP

#include "graphics_common.h"
#include "graphics_pso.h"
#include <d3d12.h>
#include <wrl/client.h>

namespace graphics {
class DynamicDescriptorHeap {
  public:
    DynamicDescriptorHeap(ID3D12Device *device, UINT num_descriptors,
                   D3D12_DESCRIPTOR_HEAP_TYPE type, UINT NodeMask);
    ID3D12DescriptorHeap &Get();
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT descriptor_index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT descriptor_index);
    void AllocateDescriptor(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &cpu_handle,
                            _Out_ UINT &descriptor_heap_index);
    void FreeDescriptor() {
		descriptors_allocated = 0;
	}
  private:
    ComPtr<ID3D12DescriptorHeap> descriptor_heap;
    UINT descriptors_allocated = 0;
    UINT descriptor_size;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor_heap_cpu_handle;
};

} // namespace graphics
#endif