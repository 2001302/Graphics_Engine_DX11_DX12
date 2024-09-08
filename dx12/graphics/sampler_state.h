#ifndef _SAMPLER_STATE
#define _SAMPLER_STATE

#include "gpu_resource.h"

namespace graphics {
class SamplerState : public GpuResource {
  public:
    SamplerState() : GpuResource(), sampler_desc_(){};
    SamplerState(D3D12_SAMPLER_DESC sampler_desc)
    {
        sampler_desc_ = sampler_desc;
    };

    void Allocate(GpuHeap *heap, UINT &index) override {
        GpuResource::Allocate(heap, index);
        GpuDevice::Get().device->CreateSampler(&sampler_desc_, cpu_handle_);
    };

  private:
    D3D12_SAMPLER_DESC sampler_desc_;
};
} // namespace graphics
#endif