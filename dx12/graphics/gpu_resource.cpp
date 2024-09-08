#include "gpu_resource.h"

namespace graphics {
void GpuResource::Allocate(GpuHeap *heap, UINT &index) {
    Allocate(heap);
    index = index_;
};

void GpuResource::Allocate(GpuHeap *heap) {
    heap->AllocateDescriptor(cpu_handle_, index_);
};

void GpuResourceList::Add(std::shared_ptr<GpuResource> buffer) {
    buffer_list_.push_back(buffer);
}

void GpuResourceList::Allocate() {
    for (int i = 0; i < buffer_list_.size(); i++) {
        if (i == 0)
            buffer_list_[i]->Allocate(heap_, index_);
        else {
            UINT index = 0;
            buffer_list_[i]->Allocate(heap_, index);
        }
    }
}

D3D12_GPU_DESCRIPTOR_HANDLE GpuResourceList::GetGpuHandle() {
    return heap_->GetGpuHandle(index_);
}
} // namespace dx12