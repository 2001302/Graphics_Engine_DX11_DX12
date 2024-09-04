#include "gpu_buffer.h"

namespace graphics {
void GpuBuffer::Allocate(GpuHeap *heap, UINT &index) {
    Allocate(heap);
    index = index_;
};

void GpuBuffer::Allocate(GpuHeap *heap) {
    heap->AllocateDescriptor(cpu_handle_, index_);
};

void GpuBufferList::Add(std::shared_ptr<GpuBuffer> buffer) {
    buffer_list_.push_back(buffer);
}

void GpuBufferList::Allocate() {
    for (int i = 0; i < buffer_list_.size(); i++) {
        if (i == 0)
            buffer_list_[i]->Allocate(heap_, index_);
        else {
            UINT index = 0;
            buffer_list_[i]->Allocate(heap_, index);
        }
    }
}

D3D12_GPU_DESCRIPTOR_HANDLE GpuBufferList::GetGpuHandle() {
    return heap_->GetGpuHandle(index_);
}
} // namespace dx12