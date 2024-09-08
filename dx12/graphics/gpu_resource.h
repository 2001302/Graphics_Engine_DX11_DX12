#ifndef _GPU_RESOURCE
#define _GPU_RESOURCE

#include "descriptor_heap.h"
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

namespace graphics {
class GpuResource {
  public:
    GpuResource()
        : resouce_(0), index_(0), cpu_handle_(D3D12_CPU_DESCRIPTOR_HANDLE()){};
    virtual void Allocate(DescriptorHeap *heap, UINT &index);

  protected:
    void Allocate(DescriptorHeap *heap);
    Microsoft::WRL::ComPtr<ID3D12Resource> resouce_;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_;
    UINT index_;
};

class GpuResourceList {
  public:
    GpuResourceList(DescriptorHeap *heap) : resource_list_(0), index_(0) { heap_ = heap; };
    void Add(std::shared_ptr<GpuResource> buffer);
    void Allocate();
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle();

  private:
    std::vector<std::shared_ptr<GpuResource>> resource_list_;
    UINT index_;
    DescriptorHeap *heap_;
};

} // namespace dx12
#endif