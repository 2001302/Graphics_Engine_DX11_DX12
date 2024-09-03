#ifndef _GPU_BUFFER
#define _GPU_BUFFER

#include "gpu_heap.h"
#include <d3d12.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

namespace dx12 {
class GpuBuffer {
  public:
    GpuBuffer()
        : buffer_(0), index_(0), cpu_handle_(D3D12_CPU_DESCRIPTOR_HANDLE()){};
    virtual void Allocate(GpuHeap *heap, UINT &index);

  protected:
    void Allocate(GpuHeap *heap);
    Microsoft::WRL::ComPtr<ID3D12Resource> buffer_;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_;
    UINT index_;
};

class GpuBufferList {
  public:
    GpuBufferList(GpuHeap *heap) : buffer_list_(0), index_(0) { heap_ = heap; };
    void Add(std::shared_ptr<GpuBuffer> buffer);
    void Allocate();
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle();

  private:
    std::vector<std::shared_ptr<GpuBuffer>> buffer_list_;
    UINT index_;
    GpuHeap *heap_;
};

} // namespace dx12
#endif