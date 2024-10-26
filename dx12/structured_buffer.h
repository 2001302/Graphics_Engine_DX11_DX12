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

    virtual void Initialize() { Util::CreateStructuredBuffer(cpu, gpu); }

    void Upload() { Util::UpdateBuffer(cpu, gpu); }
    const auto GetBuffer() { return gpu.Get(); }

    std::vector<T_ELEMENT> cpu;
    ComPtr<ID3D12Resource> gpu;
    D3D12_CPU_DESCRIPTOR_HANDLE srv_handle;
    D3D12_CPU_DESCRIPTOR_HANDLE uav_handle;
};
} // namespace graphics
#endif
