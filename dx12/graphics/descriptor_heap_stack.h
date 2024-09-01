#ifndef _DISCRIPTOR_HEAP_STACK
#define _DISCRIPTOR_HEAP_STACK

#include "graphics_core.h"

namespace dx12 {
struct DescriptorHeap {
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
    UINT heap_index;
};

class DescriptorHeapStack {
  public:
    DescriptorHeapStack(UINT num_descriptors, D3D12_DESCRIPTOR_HEAP_TYPE type,
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

    void AllocateDescriptor(_Out_ DescriptorHeap &descriptor) {
        descriptor.heap_index = descriptors_allocated;
        descriptor.cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            descriptor_heap_cpu_handle, descriptor.heap_index, descriptor_size);
        descriptors_allocated++;
    }

    // UINT AllocateBufferSrv(_In_ ID3D12Resource &resource) {
    //     UINT descriptorHeapIndex;
    //     D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    //     AllocateDescriptor(cpuHandle, descriptorHeapIndex);
    //     D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    //     srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    //     srvDesc.Buffer.NumElements =
    //         (UINT)(resource.GetDesc().Width / sizeof(UINT32));
    //     srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
    //     srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    //     srvDesc.Shader4ComponentMapping =
    //         D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    //    GpuCore::Instance().device->CreateShaderResourceView(
    //        &resource, &srvDesc, cpuHandle);
    //    return descriptorHeapIndex;
    //}

    // UINT AllocateBufferUav(_In_ ID3D12Resource &resource) {
    //     UINT descriptorHeapIndex;
    //     D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    //     AllocateDescriptor(cpuHandle, descriptorHeapIndex);
    //     D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    //     uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    //     uavDesc.Buffer.NumElements =
    //         (UINT)(resource.GetDesc().Width / sizeof(UINT32));
    //     uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    //     uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;

    //    GpuCore::Instance().device->CreateUnorderedAccessView(
    //        &resource, nullptr, &uavDesc, cpuHandle);
    //    return descriptorHeapIndex;
    //}

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT descriptor_index) {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE(
            descriptor_heap->GetGPUDescriptorHandleForHeapStart(),
            descriptor_index, descriptor_size);
    }

  private:
    ComPtr<ID3D12DescriptorHeap> descriptor_heap;
    UINT descriptors_allocated = 0;
    UINT descriptor_size;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor_heap_cpu_handle;
};
} // namespace dx12
#endif