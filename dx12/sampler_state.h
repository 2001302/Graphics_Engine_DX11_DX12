#ifndef _SAMPLER_STATE
#define _SAMPLER_STATE

#include "gpu_resource.h"

namespace graphics {
class SamplerState {
  public:
    SamplerState(){};
    static SamplerState *Create(std::vector<D3D12_SAMPLER_DESC> sampler_desc) {
        SamplerState *sampler = new SamplerState();
        sampler->Initailize(sampler_desc);
        return sampler;
    };

    void Initailize(std::vector<D3D12_SAMPLER_DESC> sampler_desc) {
        cpu_handle_.resize(sampler_desc.size());
        index_.resize(sampler_desc.size());

        for (int i = 0; i < sampler_desc.size(); i++) {
            GpuCore::Instance().GetHeap().Sampler()->AllocateDescriptor(
                cpu_handle_[i], index_[i]);
            GpuCore::Instance().GetDevice()->CreateSampler(&sampler_desc[i],
                                                           cpu_handle_[i]);
        }
    };

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() {
        return GpuCore::Instance().GetHeap().Sampler()->GetGpuHandle(
            index_.front());
    };

  private:
    std::vector<UINT> index_;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> cpu_handle_;
};
} // namespace graphics
#endif