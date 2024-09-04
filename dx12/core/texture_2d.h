#ifndef _TEXTURE2D
#define _TEXTURE2D

#include "gpu_buffer.h"
#include "image.h"
#include <filesystem>

namespace dx12 {
class Texture2D : public GpuBuffer {
  public:
    Texture2D() : GpuBuffer(){};
    Texture2D(int width, int height, DXGI_FORMAT format);
    Texture2D(Image image, ComPtr<ID3D12GraphicsCommandList> command_list);
    void Allocate(GpuHeap *heap, UINT &index) override;

  private:
    void CreateTextureHelper(Image *image, ComPtr<ID3D12Resource> &texture,
                             ComPtr<ID3D12GraphicsCommandList> command_list);
    ComPtr<ID3D12Resource> upload;
};
} // namespace dx12
#endif
