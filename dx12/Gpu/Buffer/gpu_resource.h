#ifndef _GPU_RESOURCE
#define _GPU_RESOURCE

#include "../heap/dynamic_descriptor_heap.h"
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

namespace graphics {
class GpuResource {
  public:
    GpuResource() : resource_(0), current_state_(){};

    virtual ID3D12Resource *Get() { return resource_; };
    virtual D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() {
        throw std::runtime_error("Not implemented");
    };
    virtual D3D12_RESOURCE_STATES GetCurrentState() { return current_state_; };
    virtual void SetCurrentState(D3D12_RESOURCE_STATES state) {
        current_state_ = state;
    };

  protected:
    D3D12_RESOURCE_STATES current_state_;
    ID3D12Resource *resource_;
};

class GpuResourceList {
  public:
    GpuResourceList(std::vector<GpuResource *> resources) {
        resources_ = resources;
    };
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() {
        return resources_.front()->GetGpuHandle();
    };
    GpuResource *GetResource(int index) { return resources_[index]; };

  private:
    std::vector<GpuResource *> resources_;
};

} // namespace graphics
#endif