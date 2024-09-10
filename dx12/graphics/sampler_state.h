#ifndef _SAMPLER_STATE
#define _SAMPLER_STATE

#include "gpu_resource.h"

namespace graphics {
class SamplerState : public GpuResource {
  public:
    SamplerState() : heap_(0){};
    void Create(DescriptorHeap *heap,
                std::vector<D3D12_SAMPLER_DESC> sampler_desc) {
        sampler_desc_ = sampler_desc;
        heap_ = heap;
    };

    void Allocate() override {

        cpu_handle_.resize(sampler_desc_.size());
        index_.resize(sampler_desc_.size());

        for (int i = 0; i < sampler_desc_.size(); i++) {
            heap_->AllocateDescriptor(cpu_handle_[i], index_[i]);
            GpuDevice::Get().device->CreateSampler(&sampler_desc_[i],
                                                   cpu_handle_[i]);
        }
    };

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() override {
        return heap_->GetGpuHandle(index_.front());
    };

  private:
    DescriptorHeap *heap_;
    std::vector<UINT> index_;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> cpu_handle_;
    std::vector<D3D12_SAMPLER_DESC> sampler_desc_;
};
} // namespace graphics
#endif