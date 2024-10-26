#ifndef _STRUCTUREDBUFFER
#define _STRUCTUREDBUFFER

#include "mesh_util.h"
#include <assert.h>
#include <iostream>
#include <vector>

namespace graphics {
template <typename T_ELEMENT> class StructuredBuffer {
  public:
    virtual void Initialize(const UINT numElements) {
        cpu.resize(numElements);
        Initialize();
    }

    virtual void Initialize() {
        Util::CreateStructuredBuffer(cpu, gpu);
        GpuCore::Instance().GetHeap().View()->AllocateDescriptor(handle_CPU,
                                                                 index);
        D3D12_SHADER_RESOURCE_VIEW_DESC desc_SRV = {
            DXGI_FORMAT_UNKNOWN, D3D12_SRV_DIMENSION_BUFFER,
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};
        desc_SRV.Buffer.FirstElement = 0;
        desc_SRV.Buffer.NumElements = cpu.size();
        desc_SRV.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        desc_SRV.Buffer.StructureByteStride = sizeof(T_ELEMENT);
        GpuCore::Instance().GetDevice()->CreateShaderResourceView(
            gpu.Get(), &desc_SRV, handle_CPU);
    }

    void Upload() { Util::UpdateBuffer(cpu, gpu); }
    std::vector<T_ELEMENT> &GetCpu() { return cpu; }
    const auto GetBuffer() { return gpu.Get(); }
    const auto GetHandle() {
        return GpuCore::Instance().GetHeap().View()->GetGpuHandle(index);
    }

  private:
    std::vector<T_ELEMENT> cpu;
    ComPtr<ID3D12Resource> gpu;
    UINT index;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_CPU;
};
} // namespace graphics
#endif
