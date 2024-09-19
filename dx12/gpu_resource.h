#ifndef _GPU_RESOURCE
#define _GPU_RESOURCE

#include "dynamic_descriptor_heap.h"
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

namespace graphics {
class GpuResource {
  public:
    GpuResource() {};
    virtual void Allocate(){};
    virtual D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() {
        return D3D12_GPU_DESCRIPTOR_HANDLE();
    };
};

class GpuResourceList {
  public:
    GpuResourceList(std::vector<GpuResource *> resources) {
        resources_ = resources;
    };
    void Allocate()
    {
        for (int i = 0; i < resources_.size(); i++)
			resources_[i]->Allocate();
    };
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() {
        return resources_.front()->GetGpuHandle();
    };

  private:
    std::vector<GpuResource *> resources_;
};

} // namespace graphics
#endif