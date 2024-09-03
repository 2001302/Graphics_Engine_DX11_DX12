#ifndef _DISCRIPTOR_HEAP_STACK
#define _DISCRIPTOR_HEAP_STACK

#include "graphics_core.h"

namespace dx12 {
class GpuHeap {
  public:
    GpuHeap(UINT num_descriptors, D3D12_DESCRIPTOR_HEAP_TYPE type,
                        UINT NodeMask) {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = num_descriptors;
        desc.Type = type;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = NodeMask;
        GpuCore::Instance().device->CreateDescriptorHeap(
            &desc, IID_PPV_ARGS(&descriptor_heap));

        descriptor_size =
            GpuCore::Instance().device->GetDescriptorHandleIncrementSize(type);
        descriptor_heap_cpu_handle =
            descriptor_heap->GetCPUDescriptorHandleForHeapStart();
    }

    ID3D12DescriptorHeap &GetDescriptorHeap() { return *descriptor_heap.Get(); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT descriptor_index) {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE(
            descriptor_heap->GetGPUDescriptorHandleForHeapStart(),
            descriptor_index, descriptor_size);
    }

    void AllocateDescriptor(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &cpu_handle,
                            _Out_ UINT &descriptor_heap_index) {
        descriptor_heap_index = descriptors_allocated;
        cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            descriptor_heap_cpu_handle, descriptor_heap_index, descriptor_size);
        descriptors_allocated++;
    }

  private:
    ComPtr<ID3D12DescriptorHeap> descriptor_heap;
    UINT descriptors_allocated = 0;
    UINT descriptor_size;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor_heap_cpu_handle;
};

} // namespace dx12
#endif