#ifndef _STRUCTUREDBUFFER
#define _STRUCTUREDBUFFER

#include "../../util/mesh_util.h"
#include <assert.h>
#include <iostream>
#include <vector>

namespace graphics {
template <typename T_ELEMENT> class StructuredBuffer {
  public:
    virtual void Initialize(const UINT numElements) {
        data.resize(numElements);
        Initialize();
    }

    virtual void Initialize() {
        Util::CreateStructuredBuffer(data, buffer);
        GpuCore::Instance().GetHeap().View()->AllocateDescriptor(handle_CPU,
                                                                 index);
        D3D12_SHADER_RESOURCE_VIEW_DESC desc_SRV = {
            DXGI_FORMAT_UNKNOWN, D3D12_SRV_DIMENSION_BUFFER,
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING};
        desc_SRV.Buffer.FirstElement = 0;
        desc_SRV.Buffer.NumElements = data.size();
        desc_SRV.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        desc_SRV.Buffer.StructureByteStride = sizeof(T_ELEMENT);
        GpuCore::Instance().GetDevice()->CreateShaderResourceView(
            buffer.Get(), &desc_SRV, handle_CPU);
    }

    void Upload() {
        Util::UpdateBuffer(data, buffer);
    }
    std::vector<T_ELEMENT> &GetCpu() { return data; }
    const auto GetBuffer() { return buffer.Get(); }
    const auto GetHandle() {
        return GpuCore::Instance().GetHeap().View()->GetGpuHandle(index);
    }

  private:
    std::vector<T_ELEMENT> data;
    ComPtr<ID3D12Resource> buffer;
    UINT index;
    D3D12_CPU_DESCRIPTOR_HANDLE handle_CPU;
};
} // namespace graphics
#endif
