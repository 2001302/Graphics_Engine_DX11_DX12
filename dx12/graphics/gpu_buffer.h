#ifndef _GPU_BUFFER
#define _GPU_BUFFER

#include "gpu_resource.h"

namespace graphics {
class GpuBuffer : public GpuResource{
  public:
    GpuBuffer() : GpuResource(){};
    //GpuBuffer(const wchar_t *file_name,
    //            ComPtr<ID3D12GraphicsCommandList> command_list, bool isCubeMap,
    //            bool isBrdf = false);

    void CreateAsDisplayPlane() 
    {

    }
    void Allocate(DescriptorHeap *heap, UINT &index) override {
        GpuResource::Allocate(heap, index);

    };

};
} // namespace graphics
#endif