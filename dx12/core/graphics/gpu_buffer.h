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
    virtual void Allocate(GpuHeap *heap, UINT &index) {
        Allocate(heap);
        index = index_;
    };

  protected:
    void Allocate(GpuHeap *heap) {
        heap->AllocateDescriptor(cpu_handle_, index_);
    };
    Microsoft::WRL::ComPtr<ID3D12Resource> buffer_;
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle_;
    UINT index_;
};

class GpuBufferList {
  public:
    GpuBufferList() : buffer_list_(0), index_(0){};
    void Add(std::shared_ptr<GpuBuffer> buffer) {
        buffer_list_.push_back(buffer);
    }
    void Allocate(GpuHeap *heap) {
        for (int i = 0; i < buffer_list_.size(); i++) {
            if (i == 0)
                buffer_list_[i]->Allocate(heap, index_);
            else {
                UINT index = 0;
                buffer_list_[i]->Allocate(heap, index);
            }
        }
    }
    UINT Index() { return index_; }

  private:
    std::vector<std::shared_ptr<GpuBuffer>> buffer_list_;
    UINT index_;
};

} // namespace dx12
#endif