#ifndef _DISCRIPTOR_HEAP_STACK
#define _DISCRIPTOR_HEAP_STACK

#include "graphics_core.h"

namespace dx12 {
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

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT descriptor_index) {
        return CD3DX12_GPU_DESCRIPTOR_HANDLE(
            descriptor_heap->GetGPUDescriptorHandleForHeapStart(),
            descriptor_index, descriptor_size);
    }

    UINT AllocateTextureCube(_In_ ID3D12Resource *resource) {
        UINT descriptor_heap_index;
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
        AllocateDescriptor(cpu_handle, descriptor_heap_index);

        D3D12_SHADER_RESOURCE_VIEW_DESC desc_SRV = {
            DXGI_FORMAT_UNKNOWN, D3D12_SRV_DIMENSION_TEXTURECUBE,
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

        if (resource != nullptr) {
            desc_SRV.Format = resource->GetDesc().Format;
            desc_SRV.TextureCube.MipLevels = resource->GetDesc().MipLevels;
            desc_SRV.TextureCube.MostDetailedMip = 0;
            desc_SRV.TextureCube.ResourceMinLODClamp = 0;
        }

        GpuCore::Instance().device->CreateShaderResourceView(
            resource, &desc_SRV, cpu_handle);

        return descriptor_heap_index;
    }

    UINT AllocateTexture2D(_In_ ID3D12Resource *resource) {
        UINT descriptor_heap_index;
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
        AllocateDescriptor(cpu_handle, descriptor_heap_index);

        D3D12_SHADER_RESOURCE_VIEW_DESC desc_SRV = {
            resource->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D,
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};

        if (resource != nullptr) {
            desc_SRV.Texture2D.MipLevels = resource->GetDesc().MipLevels;
        }

        GpuCore::Instance().device->CreateShaderResourceView(
            resource, &desc_SRV, cpu_handle);
        return descriptor_heap_index;
    }

  private:
    void AllocateDescriptor(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &cpu_handle,
                            _Out_ UINT &descriptor_heap_index) {
        descriptor_heap_index = descriptors_allocated;
        cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            descriptor_heap_cpu_handle, descriptor_heap_index, descriptor_size);
        descriptors_allocated++;
    }
    ComPtr<ID3D12DescriptorHeap> descriptor_heap;
    UINT descriptors_allocated = 0;
    UINT descriptor_size;
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor_heap_cpu_handle;
};

class DescriptorHeapInfo {
  public:
    DescriptorHeapInfo(UINT start_index,
                       UINT count)
    {
		this->start_index = start_index;
		this->count = count;
    };
    UINT GetIndex() { return start_index; }
  private:
    UINT start_index;
    UINT count;
};

} // namespace dx12
#endif