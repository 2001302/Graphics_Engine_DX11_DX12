#include "gpu_heap.h"

namespace dx12 {
GpuHeap::GpuHeap(UINT num_descriptors, D3D12_DESCRIPTOR_HEAP_TYPE type,
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

    // sampler : s0~s6
    std::vector<D3D12_SAMPLER_DESC> sampleStates;
    sampleStates.push_back(dx12::sampler::linearWrapSS);
    sampleStates.push_back(dx12::sampler::linearClampSS);
    sampleStates.push_back(dx12::sampler::shadowPointSS);
    sampleStates.push_back(dx12::sampler::shadowCompareSS);
    sampleStates.push_back(dx12::sampler::pointWrapSS);
    sampleStates.push_back(dx12::sampler::linearMirrorSS);
    sampleStates.push_back(dx12::sampler::pointClampSS);

    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = (UINT)sampleStates.size();
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    dx12::ThrowIfFailed(dx12::GpuCore::Instance().device->CreateDescriptorHeap(
        &samplerHeapDesc, IID_PPV_ARGS(&sampler_heap)));

    CD3DX12_CPU_DESCRIPTOR_HANDLE sampler_heap_cpu_handle(
        sampler_heap->GetCPUDescriptorHandleForHeapStart());
    UINT descriptor_size_sampler =
        dx12::GpuCore::Instance().device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    for (int i = 0; i < sampleStates.size(); ++i) {
        dx12::GpuCore::Instance().device->CreateSampler(
            &sampleStates[i], sampler_heap_cpu_handle);
        sampler_heap_cpu_handle.Offset(descriptor_size_sampler);
    }
}

ID3D12DescriptorHeap &GpuHeap::GetDescriptorHeap() {
    return *descriptor_heap.Get();
}

ID3D12DescriptorHeap &GpuHeap::GetSamplerHeap() { return *sampler_heap.Get(); }

D3D12_GPU_DESCRIPTOR_HANDLE GpuHeap::GetGpuHandle(UINT descriptor_index) {
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(
        descriptor_heap->GetGPUDescriptorHandleForHeapStart(), descriptor_index,
        descriptor_size);
}

D3D12_GPU_DESCRIPTOR_HANDLE
GpuHeap::GetSamplerGpuHandle() {
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(
        sampler_heap->GetGPUDescriptorHandleForHeapStart(), 0,
        descriptor_size_sampler);
}

void GpuHeap::AllocateDescriptor(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &cpu_handle,
                                 _Out_ UINT &descriptor_heap_index) {
    descriptor_heap_index = descriptors_allocated;
    cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        descriptor_heap_cpu_handle, descriptor_heap_index, descriptor_size);
    descriptors_allocated++;
}
} // namespace dx12