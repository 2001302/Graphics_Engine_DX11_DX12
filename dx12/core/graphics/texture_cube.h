#ifndef _TEXTURECUBE
#define _TEXTURECUBE

#include "gpu_buffer.h"
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>
#include <filesystem>

namespace dx12 {
class TextureCube : public GpuBuffer {
  public:
    TextureCube() : GpuBuffer(), isBrdf_(0){};
    TextureCube(const wchar_t *file_name,
                ComPtr<ID3D12GraphicsCommandList> command_list, bool isCubeMap,
                bool isBrdf = false);
    void Allocate(GpuHeap *heap, UINT &index) override;
  private:
      bool isBrdf_;
};
} // namespace dx12
#endif
